-- TEST stand-in for the sync: copy back what the website now holds for the test lines (values read from the website pages / API replies).
update RAW_MATERIAL_PLANNING_SUB set TICKET_NO = 'S000164714', LORRY_PLATE_NO = 'ACST201', DATE_IN = '2026-11-26 22:16:09', DATE_OUT = '2026-11-26 22:17:36' where REF_NO = 'WB-PO-0001-RM-MOP60E-1'
update RAW_MATERIAL_PLANNING_SUB set TICKET_NO = 'S000164715', LORRY_PLATE_NO = 'ACST202', DATE_IN = '2026-11-26 22:25:26' where REF_NO = 'WB-PO-0001-RM-MOP60E-2'
update DAILY_DELIVERY_PLANNING_SUB set TIME_IN = '2026-11-26 22:22:11', TIME_OUT = '2026-11-26 22:23:27' where DAILY_DELIVERY_PLANNING_ID = 5
GO
select REF_NO, TICKET_NO, DATE_IN, DATE_OUT from RAW_MATERIAL_PLANNING_SUB
select count(*) open_refs from V_ACS_RECEIVING_REF
select count(*) open_dos from V_ACS_DESPATCH_DO
