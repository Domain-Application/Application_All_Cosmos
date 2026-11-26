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
| `actual_weight` / `actual_unit` | Nett kg `m[11]` ÷ 1000 / `"MT"`. The website's Unit master has no KG (UAT: BTL, EA, LITRE, MT, PACK, PC), so a `"KG"` line is refused. |
| `bin_location`, `lot_no` | Not sent (the guide calls them optional). `BIN` is used by prime-mover weighing and `LOTNO` holds the supplier D/O No., so neither means what the API means. Without `lot_no` the website uses the weigh-out date for the GRN. **Open point:** CSL rejects every GRN without a bin ("Bin Location can't be blank", see Test results). |

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

**Utility > 39 AllCosmos Resend Pending Tickets** resends one ticket at a time:

1. A list shows every ticket flagged `N` (FIRST = still in the yard, WB / DESPATCH = completed),
   newest first, with its Ref No / DO No and both flags. The operator picks one.
2. While that ticket's 1st weigh is not accepted, the app shows the Ref No / DO No it holds and
   asks "Pick a different one from the AllCosmos list?". Yes opens the same pick list as the "..."
   button; the chosen number replaces the old one on the ticket. No keeps it.
3. The ticket is sent again: Ticket In / DO Time In for a ticket still in the yard, otherwise Ticket
   Out / DO Time Out (which first re-sends the 1st weigh if needed). "Ticket … accepted by
   AllCosmos." on success; otherwise the website's message, and the ticket stays in the list.

The website accepts a resend until the GRN / Confirm DO is Posted (guide 4.1).

## Set-up

1. Run `sql/001`–`003` (columns, log tables). Run `sql/004` once the website tables are synced
   into the database (the sync is not part of this app). Details: `sql/README.md`.
2. Fill `profile.API_URL` (base URL only), `API_USER`, `API_PASSWORD`. A blank URL turns the
   integration off. UAT: `https://allcosmos.domainsb.com.my/`. Production: to be given.
3. Until `sql/004` exists, the "…" button says the list is not available; the operator types the
   Ref No / DO No.
4. The pick lists leave out numbers already on a lorry in the yard (`FIRST.API_REF_NO`). The DO
   list shows only DOs whose Delivery Date is today (James's query).

`sql/900_TEST_ONLY_load_website_test_data.sql` creates the 11 website tables with test rows so that
`sql/004` and the pick lists can be tried before the real sync exists. **Test databases only.**
Drop those tables (block at the end of the script) before the real sync is installed.

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

## Test results (UAT, 2026-11-26)

Run against UAT from the Debug build on test database `allcosmos suasa` (torry), with the
`sql/900` test tables. Every call was accepted by the website (`isSuccess: true`); screen
recordings with the website pages after each call are in the handoff folder (`video\`).

| # | Case | Ticket | Result |
|---|---|---|---|
| 1 | Ticket In, Ref No picked with "..." | S000164716 (ACST301) | Accepted; Raw Material Planning WB-PO-0002 line 1 got ticket, lorry, Time In |
| 2 | Ticket Out | S000164716 | Accepted; 30 / 30.04 / 29.98 MT and Time Out on the line |
| 3 | DO Time In, DO picked with "..." | C000002406 (CCH4092), DO 900260001 | Accepted; lorry in the Lorry Que (Filling) |
| 4 | DO Time Out | C000002406 | Accepted; Actual Quantity 20.020 MT, Lorry Que Completed |
| 5 | Ticket In with a typed, wrong Ref No, then Resend with the right one, then Ticket Out | S000164717 (ACST302) | 1st call refused ("Ref No … is not in Raw Material Planning or Import Schedule & Costing"), ticket saved with `API_IN_OK = N`; resend with WB-PO-0002-RM-RPH26-2 accepted; Ticket Out accepted (28.12 / 28.12 / 28 MT) |

Findings:

- **CSL GRN needs `bin_location`.** Every Ticket Out reply ends with "Posting the GRN to CSL failed:
  CSL answered HTTP 400. Bin Location can't be blank". The guide calls the field optional;
  production GRNs will fail the same way. Decision needed (Kelvin / James): one fixed bin per site
  in `profile`, or a field on the 2nd-weigh screen.
- **CSL Confirm DO** needs a numeric DO No (max 9 digits). With numeric DO 900260001 it still
  failed in UAT: "CSL answered HTTP 503. Service unavailable - failed to process the request Invalid
  division" (shown on the Daily Delivery Planning page). Test-data issue on CSL's side; the
  weighbridge call itself was accepted.
- Lorry plates are stored without spaces (CCH 4092 → CCH4092), but the 2nd-weigh search uses the
  typed text: typing "CCH 4092" opens a new 1st weigh. Existing behaviour, not from this change.
- Supervisor approval (`CLogerr`, LOGERR.CPP) compares the typed password with `user.ACCESS` as
  plain text, so a normal login password is refused there. Existing behaviour.
- Ticket Out / DO Time Out is sent after the ticket print returns (IT2.CPP). If the Crystal job
  cannot open (missing `.rpt`, no Crystal runtime), wlib32's hidden print dialog never closes, the
  screen stays on "Proceeding..." and the call is not sent (seen on a test PC without Crystal).
  Normal sites with Crystal are not affected; Resend covers a ticket left unsent.
- The website's API Demo page has an API login written in its page source - reported to James.

## Not done yet

- **Profile screen fields** for API URL / user / password: `CProfile` binds controls by column
  position, so they wait for the live column check on the site database. Until then, set the
  three values with SQL (`sql/README.md`).
- **`bin_location`** - see Findings.
- **The website sync** (the 11 tables `sql/004` reads) is not this app's job and does not exist
  yet; `sql/900` stands in for it on test databases only.
