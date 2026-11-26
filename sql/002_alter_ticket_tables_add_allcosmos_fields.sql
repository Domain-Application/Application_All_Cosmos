-- AllcosmosSystem Weighbridge API - per-ticket columns on FIRST, WB and DESPATCH.
--   API_REF_NO  Ref No (receiving: Planning line / Costing container) or DO No (despatch),
--               picked by the operator at the 1st weigh. Website max length is 120.
--   API_UNIT    PO line unit (PURCHASE_ITEM_UNIT_CODE) captured with the Ref No; Ticket Out
--               quantities are converted from kg to this unit (KG or MT).
--   API_IN_OK   Y = Ticket In / DO Time In accepted, N = rejected or not reached, blank = not sent.
--   API_OUT_OK  Same for Ticket Out / DO Time Out.
-- All three tables get the same columns: the 2nd weigh copies FIRST -> WB/DESPATCH with
-- wlib32 fetch(), which matches columns by NAME, so a plain append keeps the copy correct.
-- The app reads these columns by name (GetDbFieldNo), never by position.
-- Idempotent: safe to re-run. Refuses to alter a view.

-- USE allcosmoskeluli;   -- TODO: confirm database (PROVIDER.SYS line 1)
SELECT DB_NAME() AS running_on_database;
GO
SELECT name, type_desc FROM sys.objects WHERE name IN ('FIRST', 'WB', 'DESPATCH');
GO

DECLARE @t sysname, @sql nvarchar(1000)
DECLARE c CURSOR LOCAL FAST_FORWARD FOR
    SELECT name FROM (SELECT 'FIRST' AS name UNION ALL SELECT 'WB' UNION ALL SELECT 'DESPATCH') x
OPEN c
FETCH NEXT FROM c INTO @t
WHILE @@FETCH_STATUS = 0
BEGIN
    IF OBJECT_ID('dbo.' + @t, 'U') IS NULL
        PRINT 'SKIPPED ' + @t + ': not a base table in this database (view or missing) - alter its base table instead.'
    ELSE
    BEGIN
        IF COL_LENGTH('dbo.' + @t, 'API_REF_NO') IS NULL
        BEGIN
            SET @sql = N'ALTER TABLE [dbo].[' + @t + N'] ADD [API_REF_NO] [nvarchar](120) NULL'
            EXEC (@sql)
        END
        IF COL_LENGTH('dbo.' + @t, 'API_UNIT') IS NULL
        BEGIN
            SET @sql = N'ALTER TABLE [dbo].[' + @t + N'] ADD [API_UNIT] [nvarchar](10) NULL'
            EXEC (@sql)
        END
        IF COL_LENGTH('dbo.' + @t, 'API_IN_OK') IS NULL
        BEGIN
            SET @sql = N'ALTER TABLE [dbo].[' + @t + N'] ADD [API_IN_OK] [char](1) NULL'
            EXEC (@sql)
        END
        IF COL_LENGTH('dbo.' + @t, 'API_OUT_OK') IS NULL
        BEGIN
            SET @sql = N'ALTER TABLE [dbo].[' + @t + N'] ADD [API_OUT_OK] [char](1) NULL'
            EXEC (@sql)
        END
    END
    FETCH NEXT FROM c INTO @t
END
CLOSE c
DEALLOCATE c
GO

SELECT TABLE_NAME, COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH, ORDINAL_POSITION
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_NAME IN ('FIRST', 'WB', 'DESPATCH') AND COLUMN_NAME LIKE 'API[_]%'
ORDER BY TABLE_NAME, ORDINAL_POSITION;
