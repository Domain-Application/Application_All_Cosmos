-- AllcosmosSystem Weighbridge API - config columns on [profile].
-- API_URL blank = integration off. URL is the base only (e.g. https://allcosmos.domainsb.com.my/);
-- the app appends /api/ticketin, /api/ticketout, /api/dotimein, /api/dotimeout.
-- Credentials are HTTP Basic, entered on the Profile screen - never in this script or in git.
-- Idempotent: safe to re-run. Plain append (the app reads these columns by name).

-- USE allcosmoskeluli;   -- TODO: confirm database (PROVIDER.SYS line 1)
SELECT DB_NAME() AS running_on_database;
GO
IF COL_LENGTH('dbo.profile', 'API_URL') IS NULL
    ALTER TABLE [dbo].[profile] ADD [API_URL] [nvarchar](500) NULL
GO
IF COL_LENGTH('dbo.profile', 'API_USER') IS NULL
    ALTER TABLE [dbo].[profile] ADD [API_USER] [nvarchar](50) NULL
GO
IF COL_LENGTH('dbo.profile', 'API_PASSWORD') IS NULL
    ALTER TABLE [dbo].[profile] ADD [API_PASSWORD] [nvarchar](50) NULL
GO
SELECT COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH, ORDINAL_POSITION
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_NAME = 'profile' AND COLUMN_NAME IN ('API_URL', 'API_USER', 'API_PASSWORD')
ORDER BY ORDINAL_POSITION;
