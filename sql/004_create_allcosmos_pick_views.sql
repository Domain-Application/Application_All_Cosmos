-- AllcosmosSystem Weighbridge API - pick-list views for the 1st-weigh screen.
-- Built from the queries in "AllcosmosSystem - Weighbridge Integration Guide" sections 3.1-3.3.
-- The website tables (PURCHASE_ORDER*, RAW_MATERIAL_PLANNING*, IMPORT_SCHEDULE_COSTING*,
-- DESPATCH_TICKET*, DAILY_DELIVERY_PLANNING*) are synced in by the website side, not by this app.
-- Run this AFTER they exist. If they land in another database, prefix the table names below
-- with that database (e.g. ALLCOSMOS_SYNC.dbo.PURCHASE_ORDER) - the app only reads the views.
--   V_ACS_RECEIVING_REF : selectable Planning lines + Costing containers (no DATE_OUT yet).
--   V_ACS_DESPATCH_DO   : one row per DO No, Delivery Date = today, not Time Out, not Posted.
-- Idempotent: drops and recreates the two views only.

-- USE allcosmoskeluli;   -- TODO: confirm database (PROVIDER.SYS line 1)
SELECT DB_NAME() AS running_on_database;
GO
IF OBJECT_ID('dbo.RAW_MATERIAL_PLANNING_SUB') IS NULL OR OBJECT_ID('dbo.IMPORT_SCHEDULE_COSTING_SUB') IS NULL
   OR OBJECT_ID('dbo.PURCHASE_ORDER_SUB') IS NULL OR OBJECT_ID('dbo.DAILY_DELIVERY_PLANNING_SUB') IS NULL
   OR OBJECT_ID('dbo.DESPATCH_TICKET_SUB') IS NULL OR OBJECT_ID('dbo.DESPATCH_TICKET_CONFIRMATION') IS NULL
BEGIN
    RAISERROR('Synced website tables not found in this database - views not created.', 16, 1)
    SET NOEXEC ON
END
GO

IF OBJECT_ID('dbo.V_ACS_RECEIVING_REF', 'V') IS NOT NULL DROP VIEW [dbo].[V_ACS_RECEIVING_REF]
GO
CREATE VIEW [dbo].[V_ACS_RECEIVING_REF] AS
SELECT  line.REF_NO,
        'Raw Material Planning'        AS MODULE,
        po.REF_NO                      AS PO_NO,
        poLine.SEQUENCE                AS PO_LINE,
        po.VENDOR_CODE, po.VENDOR_NAME,
        poLine.ITEM_CODE, poLine.ITEM_DESCRIPTION,
        poLine.PURCHASE_ITEM_UNIT_CODE AS PO_UNIT,
        line.DELIVERY_DATE, line.DELIVERY_QUANTITY,
        CAST(NULL AS nvarchar(50))     AS CONTAINER_NO,
        line.SITE_CODE, line.TICKET_NO, line.LORRY_PLATE_NO, line.DATE_IN
FROM    RAW_MATERIAL_PLANNING_SUB line
JOIN    RAW_MATERIAL_PLANNING planning ON planning.PKKEY = line.RAW_MATERIAL_PLANNING_ID
JOIN    PURCHASE_ORDER_SUB poLine      ON poLine.PKKEY  = planning.PURCHASE_ORDER_SUB_ID
JOIN    PURCHASE_ORDER po              ON po.REF_NO     = poLine.PURCHASE_ORDER_REF_NO
WHERE   line.REF_NO IS NOT NULL
AND     line.DATE_OUT IS NULL
UNION ALL
SELECT  container.REF_NO,
        'Import Schedule & Costing'    AS MODULE,
        po.REF_NO                      AS PO_NO,
        poLine.SEQUENCE                AS PO_LINE,
        po.VENDOR_CODE, po.VENDOR_NAME,
        poLine.ITEM_CODE, poLine.ITEM_DESCRIPTION,
        poLine.PURCHASE_ITEM_UNIT_CODE AS PO_UNIT,
        NULL                           AS DELIVERY_DATE,
        NULL                           AS DELIVERY_QUANTITY,
        container.CONTAINER_NO,
        container.SITE_CODE, container.TICKET_NO, container.LORRY_PLATE_NO, container.DATE_IN
FROM    IMPORT_SCHEDULE_COSTING_SUB container
JOIN    IMPORT_SCHEDULE_COSTING costing ON costing.PKKEY = container.IMPORT_SCHEDULE_COSTING_ID
JOIN    PURCHASE_ORDER_SUB poLine       ON poLine.PKKEY  = costing.PURCHASE_ORDER_SUB_ID
JOIN    PURCHASE_ORDER po               ON po.REF_NO     = poLine.PURCHASE_ORDER_REF_NO
WHERE   container.REF_NO IS NOT NULL
AND     container.DATE_OUT IS NULL
GO

IF OBJECT_ID('dbo.V_ACS_DESPATCH_DO', 'V') IS NOT NULL DROP VIEW [dbo].[V_ACS_DESPATCH_DO]
GO
CREATE VIEW [dbo].[V_ACS_DESPATCH_DO] AS
SELECT  ticketLine.DO_NO,
        MIN(ticket.REF_NO)                  AS DESPATCH_TICKET,
        MIN(ticket.LORRY_PLATE_NO)          AS LORRY_PLATE_NO,
        MIN(ticket.TRANSPORTER_DESCRIPTION) AS TRANSPORTER,
        MIN(planning.DELIVERY_DATE)         AS DELIVERY_DATE,
        MIN(ticketLine.CUSTOMER_NAME)       AS CUSTOMER_NAME,
        MIN(ticketLine.ITEM_DESCRIPTION)    AS ITEM_DESCRIPTION,
        SUM(ticketLine.WEIGHT)              AS WEIGHT,
        MIN(ticketLine.ITEM_UNIT_CODE)      AS ITEM_UNIT_CODE,
        COUNT(*)                            AS LINES,
        MIN(planningLine.SITE_CODE)         AS SITE_CODE,
        MAX(planningLine.TIME_IN)           AS TIME_IN
FROM    DAILY_DELIVERY_PLANNING_SUB planningLine
JOIN    DAILY_DELIVERY_PLANNING planning ON planning.PKKEY = planningLine.DAILY_DELIVERY_PLANNING_ID
JOIN    DESPATCH_TICKET_SUB ticketLine   ON ticketLine.PKKEY = planningLine.DESPATCH_TICKET_SUB_ID
JOIN    DESPATCH_TICKET ticket           ON ticket.REF_NO = ticketLine.DESPATCH_TICKET_REF_NO
LEFT JOIN DESPATCH_TICKET_CONFIRMATION confirmation ON confirmation.DESPATCH_TICKET_REF_NO = ticket.REF_NO
WHERE   ticketLine.DO_NO IS NOT NULL
AND     CAST(planning.DELIVERY_DATE AS date) = CAST(GETDATE() AS date)
AND     planningLine.TIME_OUT IS NULL
AND     (confirmation.STATUS IS NULL OR confirmation.STATUS <> 'Posted')
GROUP BY ticketLine.DO_NO
GO

SET NOEXEC OFF
GO
SELECT name, create_date FROM sys.views WHERE name IN ('V_ACS_RECEIVING_REF', 'V_ACS_DESPATCH_DO');
