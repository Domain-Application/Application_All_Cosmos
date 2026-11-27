-- AllcosmosSystem Weighbridge API - Bin Location for the receiving Ticket Out (CSL GRN needs it).
--   API_BIN           on FIRST, WB and DESPATCH: the bin picked at the 2nd weigh, sent as
--                     ticketout.bin_location (website max 15). Same columns on all three tables
--                     because the 2nd weigh copies FIRST -> WB/DESPATCH by column NAME (see 002).
--   ACS_BIN_LOCATION  the bin master and pick list. The website has no bin master, so the
--                     weighbridge keeps its own (one per site database) with CSL's bin codes.
-- Idempotent: safe to re-run. Refuses to alter a view.

-- USE [allcosmos suasa];   -- TODO: confirm database (PROVIDER.SYS line 1)
SELECT DB_NAME() AS running_on_database;
GO
SELECT name, type_desc FROM sys.objects WHERE name IN ('FIRST', 'WB', 'DESPATCH', 'ACS_BIN_LOCATION');
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
    ELSE IF COL_LENGTH('dbo.' + @t, 'API_BIN') IS NULL
    BEGIN
        SET @sql = N'ALTER TABLE [dbo].[' + @t + N'] ADD [API_BIN] [nvarchar](15) NULL'
        EXEC (@sql)
    END
    FETCH NEXT FROM c INTO @t
END
CLOSE c
DEALLOCATE c
GO

-- Same shape as the Container / Trailer masters: the app maintains it on the shared code + description
-- screen (Table > 29 More Master Setup > 294 Set AllCosmos Bin Location). Column order matters.
IF OBJECT_ID('dbo.ACS_BIN_LOCATION', 'U') IS NULL
    CREATE TABLE dbo.ACS_BIN_LOCATION (
        CODE            nvarchar(15) NOT NULL CONSTRAINT PK_ACS_BIN_LOCATION PRIMARY KEY,
        DESCRIPTION     nvarchar(50) NULL,
        REC_STATUS      char(1) NULL,
        NEWUSER         varchar(10) NULL,
        NEWTIME         datetime NULL,
        MODIFYUSER      varchar(10) NULL,
        MODIFYTIME      datetime NULL,
        [Print Counter] int NULL
    )
GO

-- Fill it on the screen above with the site's CSL bin codes (ask the CSL user for the exact codes).

SELECT TABLE_NAME, COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH
FROM INFORMATION_SCHEMA.COLUMNS
WHERE (TABLE_NAME IN ('FIRST', 'WB', 'DESPATCH') AND COLUMN_NAME = 'API_BIN') OR TABLE_NAME = 'ACS_BIN_LOCATION'
ORDER BY TABLE_NAME, ORDINAL_POSITION;
