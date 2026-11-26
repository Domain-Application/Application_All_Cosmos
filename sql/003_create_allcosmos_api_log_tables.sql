-- AllcosmosSystem Weighbridge API - audit log tables.
-- One AcsApiRequestLog row is written BEFORE each HTTP call, one AcsApiResponseLog row AFTER it
-- (also on timeout / no answer). The password is never logged.
-- REQUEST_TS / RESPONSE_TS are stamped by the app (an ORM NULL would bypass a DEFAULT).
-- Idempotent: safe to re-run.

-- USE allcosmoskeluli;   -- TODO: confirm database (PROVIDER.SYS line 1)
SELECT DB_NAME() AS running_on_database;
GO
IF OBJECT_ID('dbo.AcsApiRequestLog', 'U') IS NULL
BEGIN
    CREATE TABLE [dbo].[AcsApiRequestLog] (
        [TICKET_NO]    [nvarchar](20)   NULL,
        [ENDPOINT]     [nvarchar](30)   NULL,
        [API_URL]      [nvarchar](500)  NULL,
        [REQUEST_DATA] [nvarchar](max)  NULL,
        [REQUEST_TS]   [datetime]       NULL,
        [PKKEY]        [int] IDENTITY(1,1) NOT NULL,
        CONSTRAINT [PK_AcsApiRequestLog] PRIMARY KEY CLUSTERED ([PKKEY])
    )
END
GO
IF OBJECT_ID('dbo.AcsApiResponseLog', 'U') IS NULL
BEGIN
    CREATE TABLE [dbo].[AcsApiResponseLog] (
        [REQUEST_PKKEY] [int]            NULL,
        [STATUS_CODE]   [int]            NULL,
        [IS_SUCCESS]    [bit]            NULL,
        [MESSAGE]       [nvarchar](1000) NULL,
        [RESPONSE_DATA] [nvarchar](max)  NULL,
        [RESPONSE_TS]   [datetime]       NULL,
        [PKKEY]         [int] IDENTITY(1,1) NOT NULL,
        CONSTRAINT [PK_AcsApiResponseLog] PRIMARY KEY CLUSTERED ([PKKEY])
    )
END
GO
IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_AcsApiRequestLog_Ticket')
    CREATE INDEX [IX_AcsApiRequestLog_Ticket] ON [dbo].[AcsApiRequestLog] ([TICKET_NO], [ENDPOINT])
GO
IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_AcsApiResponseLog_Request')
    CREATE INDEX [IX_AcsApiResponseLog_Request] ON [dbo].[AcsApiResponseLog] ([REQUEST_PKKEY])
GO
SELECT name, create_date FROM sys.tables WHERE name IN ('AcsApiRequestLog', 'AcsApiResponseLog');
