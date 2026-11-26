# AllcosmosSystem weighbridge API - source spec

From James Toh's email of 2026-11-26 14:49, subject "AllcosmosSystem – Weighbridge Integration
(Ticket In / Ticket Out / DO Time In / DO Time Out)", to Kelvin. This spec is the authority: build only what
these four APIs offer.

| File | What |
|---|---|
| `AllcosmosSystem - Weighbridge Integration Guide.md` | The full guide (Chinese): flow, data to sync, pick-list rules (section 3), the four APIs and their rules (section 4), how to make your own test data on UAT (section 5), test list (5.6) |
| `AllcosmosSystem - Weighbridge API (Normal Cases).postman_collection.json` | One normal request per API; fill the collection variables `username` / `password` |

Email body (connection details):

- Base URL (UAT): `https://allcosmos.domainsb.com.my/`
- Account: `DOMAIN` (also logs in to the website, incl. the API Demo page)
- Password (UAT test account): `1234` - kept here on Kelvin's instruction (private repo). Also in
  `profile.API_PASSWORD` of the test database. Production gets its own account; do not put that one in git.

How the app implements it: `../ALLCOSMOS_API_INTEGRATION.md`. Test status and what is left: `../ACS_TEST_HANDOFF.md`.
