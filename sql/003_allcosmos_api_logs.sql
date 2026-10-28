-- 003_allcosmos_api_logs.sql
-- Database: the All Cosmos weighbridge database (allcosmoskeluli, or the site's own copy).
-- One row per AllcosmosSystem API request (written before the call) and one per response
-- (written after it returns or fails). API_NAME is the endpoint, e.g. rawmaterialticketin.
-- Safe to run more than once.

IF OBJECT_ID('dbo.AllcosmosApiRequestLog', 'U') IS NULL
BEGIN
    CREATE TABLE [dbo].[AllcosmosApiRequestLog] (
        [TICKET_NO]    [nvarchar](20)  NULL,
        [API_NAME]     [nvarchar](50)  NULL,
        [REQUEST_TS]   [datetime]      NOT NULL CONSTRAINT [DF_AllcosmosApiRequestLog_TS] DEFAULT (getdate()),
        [API_URL]      [nvarchar](500) NULL,
        [REQUEST_DATA] [nvarchar](max) NULL,
        [PKKEY]        [int] IDENTITY(1,1) NOT NULL,
        CONSTRAINT [PK_AllcosmosApiRequestLog] PRIMARY KEY CLUSTERED ([PKKEY])
    )
    CREATE INDEX [IX_AllcosmosApiRequestLog_TICKET] ON [dbo].[AllcosmosApiRequestLog] ([TICKET_NO], [API_NAME])
END
GO

IF OBJECT_ID('dbo.AllcosmosApiResponseLog', 'U') IS NULL
BEGIN
    CREATE TABLE [dbo].[AllcosmosApiResponseLog] (
        [REQUEST_PKKEY] [int]            NOT NULL,
        [RESPONSE_TS]   [datetime]       NOT NULL CONSTRAINT [DF_AllcosmosApiResponseLog_TS] DEFAULT (getdate()),
        [STATUS_CODE]   [int]            NULL,
        [IS_SUCCESS]    [bit]            NOT NULL CONSTRAINT [DF_AllcosmosApiResponseLog_Success] DEFAULT (0),
        [RESPONSE_DATA] [nvarchar](max)  NULL,
        [ERROR_MESSAGE] [nvarchar](1000) NULL,
        [PKKEY]         [int] IDENTITY(1,1) NOT NULL,
        CONSTRAINT [PK_AllcosmosApiResponseLog] PRIMARY KEY CLUSTERED ([PKKEY]),
        CONSTRAINT [FK_AllcosmosApiResponseLog_Request] FOREIGN KEY ([REQUEST_PKKEY])
            REFERENCES [dbo].[AllcosmosApiRequestLog] ([PKKEY])
    )
    CREATE INDEX [IX_AllcosmosApiResponseLog_REQUEST] ON [dbo].[AllcosmosApiResponseLog] ([REQUEST_PKKEY])
END
GO
