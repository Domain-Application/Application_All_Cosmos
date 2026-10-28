-- 001_detail_add_do_qty_bin.sql
-- Database: the All Cosmos weighbridge database (allcosmoskeluli, or the site's own copy).
-- Adds DO_QTY (Delivery Order Quantity, kg) and BIN_LOCATION to the detail-line tables
-- FDATA, WBDATA and DBDATA, just above PKKEY (field numbers 24 and 25, PKKEY becomes 26).
-- The three tables must keep the same columns: lines are copied FDATA -> WBDATA / DBDATA
-- at the second weight. Each table is rebuilt (SQL Server can only append columns), with
-- its primary key, indexes, foreign keys and dependent views/procedures restored.
-- Safe to run more than once: a table that already has DO_QTY is skipped.

SET NOCOUNT ON
SET XACT_ABORT ON
GO

DECLARE @tables TABLE (n int IDENTITY, name sysname)
INSERT @tables (name) VALUES ('FDATA')
INSERT @tables (name) VALUES ('WBDATA')
INSERT @tables (name) VALUES ('DBDATA')

DECLARE @i int, @t sysname
SET @i = 1
WHILE @i <= 3
BEGIN
    SELECT @t = name FROM @tables WHERE n = @i
    SET @i = @i + 1

    IF OBJECT_ID('dbo.' + @t, 'U') IS NULL
    BEGIN
        RAISERROR('%s is not a table in this database (missing or a view) - stopped.', 16, 1, @t)
        RETURN
    END
    IF COL_LENGTH('dbo.' + @t, 'DO_QTY') IS NOT NULL
    BEGIN
        PRINT @t + ': DO_QTY already exists, skipped.'
        CONTINUE
    END

    DECLARE @obj int, @anchor int
    SELECT @obj = OBJECT_ID('dbo.' + @t), @anchor = NULL
    SELECT @anchor = column_id FROM sys.columns WHERE object_id = @obj AND name = 'PKKEY'
    IF @anchor IS NULL OR @anchor <> (SELECT MAX(column_id) FROM sys.columns WHERE object_id = @obj)
    BEGIN
        RAISERROR('%s: PKKEY is not the last column - stopped, check the table by hand.', 16, 1, @t)
        RETURN
    END

    -- Column list in the current order, with types read from the live table.
    DECLARE @colsBefore nvarchar(max), @listBefore nvarchar(max), @pkDef nvarchar(max)
    SELECT @colsBefore = N'', @listBefore = N''
    SELECT
        @colsBefore = @colsBefore + N'[' + c.name + N'] ' + UPPER(ty.name) +
            CASE
                WHEN ty.name IN ('nvarchar','nchar') AND c.max_length = -1 THEN N'(MAX)'
                WHEN ty.name IN ('nvarchar','nchar') THEN N'(' + CAST(c.max_length/2 AS nvarchar(10)) + N')'
                WHEN ty.name IN ('varchar','char','binary','varbinary') AND c.max_length = -1 THEN N'(MAX)'
                WHEN ty.name IN ('varchar','char','binary','varbinary') THEN N'(' + CAST(c.max_length AS nvarchar(10)) + N')'
                WHEN ty.name IN ('decimal','numeric') THEN N'(' + CAST(c.precision AS nvarchar(10)) + N',' + CAST(c.scale AS nvarchar(10)) + N')'
                ELSE N''
            END + CASE WHEN c.is_nullable = 0 THEN N' NOT NULL' ELSE N' NULL' END + N',' + CHAR(13) + CHAR(10),
        @listBefore = @listBefore + N'[' + c.name + N'],'
    FROM sys.columns c JOIN sys.types ty ON ty.user_type_id = c.user_type_id
    WHERE c.object_id = @obj AND c.column_id < @anchor
    ORDER BY c.column_id

    -- Indexes (PK included) and outgoing foreign keys, scripted before the drop.
    DECLARE @restore nvarchar(max)
    SET @restore = N''
    SELECT @restore = @restore +
        CASE WHEN i.is_primary_key = 1
             THEN N'ALTER TABLE [dbo].[' + @t + N'] ADD CONSTRAINT [' + i.name + N'] PRIMARY KEY ' + i.type_desc COLLATE DATABASE_DEFAULT + N' ('
             ELSE N'CREATE ' + CASE WHEN i.is_unique = 1 THEN N'UNIQUE ' ELSE N'' END + i.type_desc COLLATE DATABASE_DEFAULT + N' INDEX [' + i.name + N'] ON [dbo].[' + @t + N'] ('
        END +
        STUFF((SELECT N',[' + c.name + N']' + CASE WHEN ic.is_descending_key = 1 THEN N' DESC' ELSE N'' END
               FROM sys.index_columns ic JOIN sys.columns c ON c.object_id = ic.object_id AND c.column_id = ic.column_id
               WHERE ic.object_id = i.object_id AND ic.index_id = i.index_id AND ic.is_included_column = 0
               ORDER BY ic.key_ordinal FOR XML PATH('')), 1, 1, N'') + N');' + CHAR(13) + CHAR(10)
    FROM sys.indexes i
    WHERE i.object_id = @obj AND i.name IS NOT NULL
    ORDER BY i.is_primary_key DESC, i.name

    SELECT @restore = @restore +
        N'ALTER TABLE [dbo].[' + @t + N'] WITH NOCHECK ADD CONSTRAINT [' + fk.name + N'] FOREIGN KEY (' +
        STUFF((SELECT N',[' + COL_NAME(fkc.parent_object_id, fkc.parent_column_id) + N']'
               FROM sys.foreign_key_columns fkc WHERE fkc.constraint_object_id = fk.object_id
               ORDER BY fkc.constraint_column_id FOR XML PATH('')), 1, 1, N'') +
        N') REFERENCES [dbo].[' + OBJECT_NAME(fk.referenced_object_id) + N'] (' +
        STUFF((SELECT N',[' + COL_NAME(fkc.referenced_object_id, fkc.referenced_column_id) + N']'
               FROM sys.foreign_key_columns fkc WHERE fkc.constraint_object_id = fk.object_id
               ORDER BY fkc.constraint_column_id FOR XML PATH('')), 1, 1, N'') + N')' +
        CASE WHEN fk.update_referential_action_desc <> 'NO_ACTION' THEN N' ON UPDATE ' + REPLACE(fk.update_referential_action_desc, '_', ' ') COLLATE DATABASE_DEFAULT ELSE N'' END +
        CASE WHEN fk.delete_referential_action_desc <> 'NO_ACTION' THEN N' ON DELETE ' + REPLACE(fk.delete_referential_action_desc, '_', ' ') COLLATE DATABASE_DEFAULT ELSE N'' END +
        N';' + CHAR(13) + CHAR(10)
    FROM sys.foreign_keys fk WHERE fk.parent_object_id = @obj

    IF EXISTS (SELECT 1 FROM sys.foreign_keys WHERE referenced_object_id = @obj)
       OR EXISTS (SELECT 1 FROM sys.triggers WHERE parent_id = @obj)
       OR EXISTS (SELECT 1 FROM sys.sql_expression_dependencies WHERE referenced_id = @obj AND is_schema_bound_reference = 1)
    BEGIN
        RAISERROR('%s has incoming foreign keys, triggers or schema-bound objects - stopped, check by hand.', 16, 1, @t)
        RETURN
    END

    DECLARE @sql nvarchar(max), @new sysname, @seed bigint
    SELECT @new = @t + N'_NEW', @seed = CAST(IDENT_CURRENT('dbo.' + @t) AS bigint)

    BEGIN TRANSACTION

    SET @sql = N'CREATE TABLE [dbo].[' + @new + N'] (' + CHAR(13) + CHAR(10) + @colsBefore +
               N'[DO_QTY] DECIMAL(10,2) NULL,' + CHAR(13) + CHAR(10) +
               N'[BIN_LOCATION] NVARCHAR(40) NULL,' + CHAR(13) + CHAR(10) +
               N'[PKKEY] INT IDENTITY(1,1) NOT NULL)'
    EXEC sp_executesql @sql

    SET @sql = N'SET IDENTITY_INSERT [dbo].[' + @new + N'] ON;' +
               N'INSERT INTO [dbo].[' + @new + N'] (' + @listBefore + N'[PKKEY]) SELECT ' + @listBefore + N'[PKKEY] FROM [dbo].[' + @t + N'];' +
               N'SET IDENTITY_INSERT [dbo].[' + @new + N'] OFF;'
    EXEC sp_executesql @sql
    -- keep the running PKKEY, so numbers of deleted lines are not given out again
    SET @sql = N'DBCC CHECKIDENT (''dbo.' + @new + N''', RESEED, ' + CAST(@seed AS nvarchar(20)) + N') WITH NO_INFOMSGS'
    EXEC sp_executesql @sql

    SET @sql = N'DROP TABLE [dbo].[' + @t + N']'
    EXEC sp_executesql @sql
    SET @sql = N'dbo.' + @new
    EXEC sp_rename @sql, @t
    EXEC sp_executesql @restore

    COMMIT TRANSACTION
    PRINT @t + ': DO_QTY and BIN_LOCATION added above PKKEY.'
END
GO

-- Views, functions and procedures that read the three tables cache their column lists.
DECLARE @m nvarchar(600)
DECLARE mods CURSOR LOCAL FAST_FORWARD FOR
    SELECT DISTINCT QUOTENAME(SCHEMA_NAME(o.schema_id)) + '.' + QUOTENAME(o.name)
    FROM sys.sql_expression_dependencies d JOIN sys.objects o ON o.object_id = d.referencing_id
    WHERE d.referenced_id IN (OBJECT_ID('dbo.FDATA'), OBJECT_ID('dbo.WBDATA'), OBJECT_ID('dbo.DBDATA'))
OPEN mods
FETCH NEXT FROM mods INTO @m
WHILE @@FETCH_STATUS = 0
BEGIN
    BEGIN TRY
        EXEC sp_refreshsqlmodule @m
    END TRY
    BEGIN CATCH
        PRINT 'Could not refresh ' + @m + ': ' + ERROR_MESSAGE()
    END CATCH
    FETCH NEXT FROM mods INTO @m
END
CLOSE mods
DEALLOCATE mods
GO

-- Check: the last three columns of each table must read DO_QTY, BIN_LOCATION, PKKEY (24, 25, 26).
SELECT OBJECT_NAME(object_id) AS [table], column_id, name
FROM sys.columns
WHERE object_id IN (OBJECT_ID('dbo.FDATA'), OBJECT_ID('dbo.WBDATA'), OBJECT_ID('dbo.DBDATA'))
  AND name IN ('DO_QTY', 'BIN_LOCATION', 'PKKEY')
ORDER BY 1, 2
GO
