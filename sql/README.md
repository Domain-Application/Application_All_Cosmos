# SQL changes - AllcosmosSystem API and receiving DO Qty / Bin Location

Run these against the **weighbridge database** the site's `PROVIDER.SYS` line 1 points to
(`allcosmoskeluli` at Keluli; the Suasa site's own database there). Install the new
`menu.exe` only after the scripts have run. All three scripts are safe to run twice.

| Order | Script | What it does |
|---|---|---|
| 1 | `001_detail_add_do_qty_bin.sql` | Adds `DO_QTY` (DECIMAL 10,2, kg) and `BIN_LOCATION` (NVARCHAR 40) to `FDATA`, `WBDATA` and `DBDATA`, just above `PKKEY`. It rebuilds each table, keeps every row, the running `PKKEY`, the primary key, the indexes and the foreign keys, and refreshes the views and procedures that read them. Takes a few seconds per 100,000 lines. |
| 2 | `002_profile_add_allcosmos_api.sql` | Adds `API_URL`, `API_USERNAME`, `API_PASSWORD`, `SITE_CODE` to `PROFILE` (at the end). |
| 3 | `003_allcosmos_api_logs.sql` | Creates `AllcosmosApiRequestLog` and `AllcosmosApiResponseLog`. |

Take a database backup before script 1: it drops and recreates the three detail tables
inside a transaction.

## Check

Script 1 and 2 end with a query. Expected:

- `FDATA`, `WBDATA`, `DBDATA`: `DO_QTY` 24, `BIN_LOCATION` 25, `PKKEY` 26.
- `PROFILE`: `API_URL` 24, `API_USERNAME` 25, `API_PASSWORD` 26, `SITE_CODE` 27.

If script 1 stops with "PKKEY is not the last column" or "incoming foreign keys, triggers
or schema-bound objects", the site's table is not the shape this script expects. Stop and
call the programmer; nothing has been changed.

## Set up the connection (in the program)

Menu **3. Utility > Company Profile**, group **AllcosmosSystem**:

| Box | Value |
|---|---|
| API URL | `https://allcosmos.domainsb.com.my` (the base address only; the program adds `/api/...`) |
| Username / Password | the login AllcosmosSystem gave for the weighbridge |
| Site | **Keluli** or **Suasa** |

Leave **API URL** blank to switch the integration off (nothing is sent, the status line and
the Resend button are hidden).

## What a wrong setting looks like

The ticket screen (menus 11 and 12) shows the last result under the grid, e.g.
`AllcosmosSystem rawmaterialticketin: FAILED (2026-10-28 08:05:00) - ...`:

| Message | Meaning |
|---|---|
| `HTTP status 401` | wrong username or password |
| `A connection with the server could not be established` | wrong API URL, or no internet on that PC |
| `Site (Keluli / Suasa) is not set in Company Profile.` | pick the site in Company Profile |
| any other text | AllcosmosSystem's own reason (e.g. the PO / delivery date does not match the planning); fix the planning, then press **Resend to AllcosmosSystem** |

Every call is kept in `AllcosmosApiRequestLog` / `AllcosmosApiResponseLog` (join on
`REQUEST_PKKEY = PKKEY`, filter on `TICKET_NO`).
