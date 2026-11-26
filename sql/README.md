# sql/ - AllcosmosSystem weighbridge API scripts

Run on the app database (PROVIDER.SYS line 1). Every script is idempotent and starts with
`SELECT DB_NAME()` so you can see where it ran. Claude does not run these.

| Script | What | When |
|---|---|---|
| `001_alter_profile_add_allcosmos_api.sql` | `profile.API_URL`, `API_USER`, `API_PASSWORD` | Now |
| `002_alter_ticket_tables_add_allcosmos_fields.sql` | `API_REF_NO`, `API_UNIT`, `API_IN_OK`, `API_OUT_OK` on FIRST, WB, DESPATCH | Now |
| `003_create_allcosmos_api_log_tables.sql` | `AcsApiRequestLog`, `AcsApiResponseLog` | Now |
| `004_create_allcosmos_pick_views.sql` | `V_ACS_RECEIVING_REF`, `V_ACS_DESPATCH_DO` (James's queries) | After the website tables are synced in. Stops with an error if they are missing. |

Before 002, check the three tables are real tables, not views (the script prints `type_desc` and
skips a view).

## Turn the integration on

Fill the values in (do not save the password in any file or in git):

```sql
update profile set API_URL = 'https://allcosmos.domainsb.com.my/',   -- UAT
                   API_USER = '<account>', API_PASSWORD = '<password>'
```

`API_URL = NULL` (or blank) turns it off. Restart the app after changing it.

## Check the calls

```sql
select top 50 q.TICKET_NO, q.ENDPOINT, q.REQUEST_TS, s.IS_SUCCESS, s.MESSAGE, q.REQUEST_DATA
from AcsApiRequestLog q left join AcsApiResponseLog s on s.REQUEST_PKKEY = q.PKKEY
order by q.PKKEY desc

select 'FIRST' t, TICKET, API_REF_NO, API_IN_OK, API_OUT_OK from [first] where API_IN_OK = 'N'
union all select 'WB', TICKET, API_REF_NO, API_IN_OK, API_OUT_OK from wb where 'N' in (API_IN_OK, API_OUT_OK)
union all select 'DESPATCH', TICKET, API_REF_NO, API_IN_OK, API_OUT_OK from despatch where 'N' in (API_IN_OK, API_OUT_OK)
```
