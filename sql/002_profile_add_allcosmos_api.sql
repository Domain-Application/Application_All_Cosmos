-- 002_profile_add_allcosmos_api.sql
-- Database: the All Cosmos weighbridge database (allcosmoskeluli, or the site's own copy).
-- AllcosmosSystem API settings on the Company Profile (menu 37). PROFILE has no PKKEY, so the
-- columns are appended after the audit block: field numbers 24-27 (IDC_EDIT24-27 on IDD_PROFILE).
-- SITE_CODE is KELULI or SUASA. Safe to run more than once.

IF COL_LENGTH('dbo.PROFILE', 'API_URL') IS NULL
    ALTER TABLE [dbo].[PROFILE] ADD [API_URL] [nvarchar](500) NULL
GO
IF COL_LENGTH('dbo.PROFILE', 'API_USERNAME') IS NULL
    ALTER TABLE [dbo].[PROFILE] ADD [API_USERNAME] [nvarchar](50) NULL
GO
IF COL_LENGTH('dbo.PROFILE', 'API_PASSWORD') IS NULL
    ALTER TABLE [dbo].[PROFILE] ADD [API_PASSWORD] [nvarchar](50) NULL
GO
IF COL_LENGTH('dbo.PROFILE', 'SITE_CODE') IS NULL
    ALTER TABLE [dbo].[PROFILE] ADD [SITE_CODE] [nvarchar](20) NULL
GO

DECLARE @m nvarchar(600)
DECLARE mods CURSOR LOCAL FAST_FORWARD FOR
    SELECT DISTINCT QUOTENAME(SCHEMA_NAME(o.schema_id)) + '.' + QUOTENAME(o.name)
    FROM sys.sql_expression_dependencies d JOIN sys.objects o ON o.object_id = d.referencing_id
    WHERE d.referenced_id = OBJECT_ID('dbo.PROFILE')
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

-- Check: API_URL, API_USERNAME, API_PASSWORD, SITE_CODE at 24-27.
SELECT column_id, name FROM sys.columns
WHERE object_id = OBJECT_ID('dbo.PROFILE') AND column_id >= 23 ORDER BY column_id
GO
