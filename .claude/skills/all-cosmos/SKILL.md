---
name: all-cosmos
description: >-
  Project facts for Application\Weighing\Application_All_Cosmos (Application_All_Cosmos; C++ MFC,
  ships as VC6): build and release, the database it points at, and what is specific to this repo.
  Use for any build, release, test, crash or data question in this repo, before the general
  skills. Add new knowledge that is true only for this repo here (see the skill-maintenance
  skill).
---

# Application_All_Cosmos - project facts

General how-to lives in the shared skills; this file holds only what is true for this repo.
Index of all repos and their project skills: the `domain-repos` skill.

## Repo

- Path (KELVIN): `E:\Dev Container\Git Repo\Application\Weighing\Application_All_Cosmos`
- Remote: https://github.com/Domain-Application/Application_All_Cosmos.git; branch at survey: `main`
- Last commit at survey: 2026-07-20 Rectified some bugs again
- Stack: C++ MFC, ships as VC6
- Shared skills that already use this repo as an example: wlib32-api

## Build and release

- Project files: `.dsp` (VC6).
- **Ships as VC6**: the newest exe `wb20260720.exe (2026-07-20)` loads `MFC42.DLL` (checked 2026-11-12). Build with VC6 - `vc6-build-debug` skill.
- Exes in the repo root (newest first): wb20260720.exe, wb20260619.exe, wb20260612.exe
- Hand-off folders seen: ToBeUpdate20260102

## Database

- PROVIDER.SYS: allcosmoskeluli on torry (SQL login) - as found in the repo. **Check whether this is a test or a customer/production server before running anything that writes.**

## Known incidents and decisions

- 2026-11-26 AllcosmosSystem weighbridge API (James Toh's guide) added: `AcsWeighbridgeApi` + `AcsTicketMapping`, called after the 1st weigh (IT1.CPP `OnFirstWt`) and 2nd weigh (IT2.CPP `OnSecond`); resend via Utility > 3.9. New columns/tables in `sql/001-004`. Business doc `docs/ALLCOSMOS_API_INTEGRATION.md`. UAT base URL `https://allcosmos.domainsb.com.my/` (credentials only in `profile`, never in git). The sync of the website tables into the DB is NOT this app's job.
- 2026-11-26 torry is unreachable while Fortinet VPN is on. torry has no `allcosmoskeluli`; its All Cosmos copy is `allcosmos suasa` (see learned.md), where sql/001-003 were run.

<!-- Facts above were read from the repo on 2026-11-12 by a survey script (project files, PROVIDER.SYS/appsettings without credentials, root files). -->
