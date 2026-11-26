# AllcosmosSystem weighbridge integration

The weighbridge app reports every weighing to the AllcosmosSystem website through four APIs.
The source spec is James Toh's "AllcosmosSystem – Weighbridge Integration Guide" and Postman
collection (email of 2026-11-26). That guide is the authority: this app implements exactly what
those four APIs offer and nothing more (no cancel, no units other than KG/MT).

## What is sent, and when

| Weighing | Moment in the app | API | Body |
|---|---|---|---|
| Receiving, 1st weigh | `CSecurity::OnFirstWt()` after `AddNew()` on FIRST (IT1.CPP) | `POST /api/ticketin` | `ref_no`, `ticket_no`, `lorry_no`, `date_in` |
| Receiving, 2nd weigh | `CSecurity::OnSecond()` after the ticket is saved to WB and the lines moved (IT2.CPP) | `POST /api/ticketout` | `ticket_no`, `date_out`, `received_qty`, `weighbridge_qty`, `supplier_delivery_qty` |
| Despatch, 1st weigh | as receiving, FIRST with `[Print Counter]` = 2 | `POST /api/dotimein` | `do_no`, `date_in` |
| Despatch, 2nd weigh | as receiving, saved to DESPATCH | `POST /api/dotimeout` | `do_no`, `date_out`, `actual_weight`, `actual_unit` |

The weighing is always saved first. The call comes after it and never stops the lorry.

### Field mapping

| API field | Source |
|---|---|
| `ref_no` / `do_no` | `API_REF_NO`, picked or typed on the 1st-weigh screen ("AllCosmos Ref No." / "AllCosmos DO No.") |
| `ticket_no` | `TICKET` (K… receiving) |
| `lorry_no` | `LORRY` |
| `date_in` / `date_out` | `TIMEIN` / `TIMEOUT`, read with `CONVERT(varchar(19), …, 120)` → `yyyy-MM-dd HH:mm:ss` |
| `received_qty` | Nett weight after wastage, `compute_weight()` `m[11]` (the app's "theoretical weight") |
| `weighbridge_qty` | \|2nd weight − 1st weight\|, `m[5]` − `m[2]` (prime-mover adjusted, before wastage) |
| `supplier_delivery_qty` | Supplier Wt `SWT` (`m[0]`) |
| (all three receiving quantities) | kg divided by the PO unit: KG ÷ 1, MT ÷ 1000. Any other unit: not sent, ticket flagged N |
| `actual_weight` / `actual_unit` | Nett kg `m[11]` / `"KG"` |
| `bin_location`, `lot_no` | Not sent (optional). `BIN` is used by prime-mover weighing and `LOTNO` holds the supplier D/O No., so neither means what the API means. Without `lot_no` the website uses the weigh-out date for the GRN. |

The PO unit (`API_UNIT`) comes with the Ref No from the pick list. If the Ref No was typed, the
unit is looked up in `V_ACS_RECEIVING_REF` at Ticket Out.

### Success and failure

Only `isSuccess` in the reply counts. The site answers HTTP 200 even for errors and for a wrong
password (checked against UAT on 2026-11-26), so the app sends the `Authorization: Basic` header
itself.

- Accepted: `API_IN_OK` / `API_OUT_OK` = `Y`.
- Rejected, no answer, blank Ref No or unknown unit: the flag is `N`, the website's message is
  shown once and written to the `status<yymmdd>.trc` log (`log_proceeding`).
- Ticket Out / DO Time Out first re-sends the 1st weigh if it was not accepted.
- GRN and Confirm DO to CSL are the website's job. Their result is in the message text only; the
  weighbridge does nothing about it (guide section 4.3 / 4.5).

### Resend

**Utility > 39 AllCosmos Resend Pending Tickets** goes through every ticket flagged `N` (yard and
completed). While a ticket's 1st weigh is not yet accepted it first shows its Ref No / DO No in a
box so the operator can correct it (Cancel stops). It then resends and ends with "n accepted,
m still pending". The website accepts a resend until the GRN / Confirm DO is Posted (guide 4.1).

## Set-up

1. Run `sql/001`–`003` (columns, log tables). Run `sql/004` once the website tables are synced
   into the database (the sync is not part of this app). Details: `sql/README.md`.
2. Fill `profile.API_URL` (base URL only), `API_USER`, `API_PASSWORD`. A blank URL turns the
   integration off. UAT: `https://allcosmos.domainsb.com.my/`. Production: to be given.
3. Until `sql/004` exists, the "…" button says the list is not available; the operator types the
   Ref No / DO No.

## Audit trail

`AcsApiRequestLog` (one row before each call: ticket, endpoint, URL, JSON) and
`AcsApiResponseLog` (one row after: HTTP status, isSuccess, message, raw body). The password is
never logged.

```sql
select q.TICKET_NO, q.ENDPOINT, q.REQUEST_TS, s.IS_SUCCESS, s.MESSAGE
from AcsApiRequestLog q left join AcsApiResponseLog s on s.REQUEST_PKKEY = q.PKKEY
order by q.PKKEY desc
```

## Testing

- UAT data: create it yourself on the website's **API Demo** page (guide section 5): a new PO
  (`WB-PO-0001`…), Raw Material Planning / Import Schedule & Costing, a Despatch Ticket
  (`WB-DT-0001`…) and a Daily Delivery Planning dated today.
- Run the guide's test list (section 5.6), receiving 1–9 and despatch 1–5, through the real
  weighing screens. Check the website pages in guide 5.4 and the two log tables.
- GRN / Confirm DO failures in UAT are normal (the test data does not exist in CSL).

## Not done yet

- **Profile screen fields** for API URL / user / password: `CProfile` binds controls by column
  position, so they wait for the live column check on the site database. Until then, set the
  three values with SQL (`sql/README.md`).
- **Pick list** (`DB::browse` over the view) is untested until the synced tables exist.
