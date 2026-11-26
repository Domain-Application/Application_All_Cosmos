# AllcosmosSystem – Weighbridge 对接指南

给 Weighing System 的开发者。说明整个流程、Weighing System 要做什么、要调用的 API、每个 API 的条件，以及**怎么在测试环境自己做测试资料**。

附件：`AllcosmosSystem - Weighbridge API (Normal Cases).postman_collection.json`（四个 API 各一个正常例子）。

名词：
- **AllcosmosSystem**：我们的网站（下文简称“网站”）。
- **CSL**：客户的 ERP。
- **Weighing System**：你们的过磅系统。
- **第一磅 / 第二磅**：一辆车进厂过磅一次，出厂再过磅一次。

---

## 1. 整体流程

### 1.1 进货（Receiving）

```
CSL ──POST Purchase Order──▶ 网站
                              │ 用户在网站上从 PO line 建立：
                              │   - Raw Material Planning（本地进货，按车次分 line）
                              │   - Import Schedule & Costing（进口货柜，按 container 分 line）
                              │   每个 line / container 保存时自动生成一个 Ref No
                              ▼
            网站资料 sync 到 Weighing System（PO、Planning、Costing）
                              ▼
Weighing System：操作员选择 Planning line 或 Costing container（不是选 PO line）
   第一磅 ──POST /api/ticketin──▶ 网站（记录 Ticket No、车牌、进厂时间）
   第二磅 ──POST /api/ticketout─▶ 网站（记录数量、出厂时间）
                              │ 网站存好后，立即 call CSL Create_GRN 通知 CSL 收货
                              ▼
                             CSL
```

### 1.2 出货（Despatch）

```
CSL ──POST Despatch Ticket──▶ 网站（每个 ticket line 有一个 DO No）
                              │ 用户在网站上从 Despatch Ticket 建立 Daily Delivery Planning，
                              │ 并填 Delivery Date
                              ▼
            网站资料 sync 到 Weighing System（Despatch Ticket、Daily Delivery Planning）
                              ▼
Weighing System：操作员选择 DO No
   第一磅 ──POST /api/dotimein──▶ 网站（记录进厂时间，更新 Lorry Que）
   第二磅 ──POST /api/dotimeout─▶ 网站（记录出厂时间、实际重量）
                              │ 当这张 Despatch Ticket 的所有 line 都有 Time Out，
                              │ 网站 call CSL Confirm_DO 通知 CSL 出货完成
                              ▼
                             CSL
```

---

## 2. Weighing System 要做的事（清单）

1. 接收网站 sync 过来的资料（第 3 节），按第 3 节的条件列出**可以选择的** Planning line / Costing container / DO No。
2. 进货：操作员选择一个 Ref No → 第一磅 call `ticketin` → 第二磅 call `ticketout`。
3. 出货：操作员选择一个 DO No → 第一磅 call `dotimein` → 第二磅 call `dotimeout`。
4. `ticketout` 的三个数量**先换算成 PO line 的单位**再送（第 4.3 节）。
5. 每次 call 都检查回复的 `isSuccess`；`false` 时把 `message` 显示给操作员，不要当作已完成。
6. 记录每次的 request 和 response（出问题时双方对照）。
7. 不需要 call CSL。GRN 和 Confirm DO 由网站负责，失败也由网站这边重发。

---

## 3. Sync 过来的资料，以及哪些可以选

数据库是 SQL Server。主键列都叫 `PKKEY`（int）。时间列是 `datetime2`，只有日期的是 `date`。

### 3.1 进货：可以选的 Raw Material Planning line

| 表 | 用途 |
|---|---|
| `RAW_MATERIAL_PLANNING_SUB` | Planning line，有 `REF_NO` |
| `RAW_MATERIAL_PLANNING` | Planning 表头，`PURCHASE_ORDER_SUB_ID` 指向 PO line |
| `PURCHASE_ORDER_SUB` | PO line（料号、单位） |
| `PURCHASE_ORDER` | PO 表头（供应商） |

```sql
SELECT  line.REF_NO,
        'Raw Material Planning'      AS MODULE,
        po.REF_NO                    AS PO_NO,
        poLine.SEQUENCE              AS PO_LINE,
        po.VENDOR_CODE, po.VENDOR_NAME,
        poLine.ITEM_CODE, poLine.ITEM_DESCRIPTION,
        poLine.PURCHASE_ITEM_UNIT_CODE AS PO_UNIT,
        line.DELIVERY_DATE, line.DELIVERY_QUANTITY, line.SITE_CODE,
        line.TICKET_NO, line.LORRY_PLATE_NO, line.DATE_IN
FROM    RAW_MATERIAL_PLANNING_SUB line
JOIN    RAW_MATERIAL_PLANNING planning ON planning.PKKEY = line.RAW_MATERIAL_PLANNING_ID
JOIN    PURCHASE_ORDER_SUB poLine      ON poLine.PKKEY  = planning.PURCHASE_ORDER_SUB_ID
JOIN    PURCHASE_ORDER po              ON po.REF_NO     = poLine.PURCHASE_ORDER_REF_NO
WHERE   line.REF_NO IS NOT NULL
AND     line.DATE_OUT IS NULL
```

### 3.2 进货：可以选的 Import Schedule & Costing container

| 表 | 用途 |
|---|---|
| `IMPORT_SCHEDULE_COSTING_SUB` | Container，有 `REF_NO`、`CONTAINER_NO` |
| `IMPORT_SCHEDULE_COSTING` | Costing 表头，`PURCHASE_ORDER_SUB_ID` 指向 PO line，`BL_NO` |
| `PURCHASE_ORDER_SUB` / `PURCHASE_ORDER` | 同上 |

```sql
SELECT  container.REF_NO,
        'Import Schedule & Costing'  AS MODULE,
        po.REF_NO                    AS PO_NO,
        poLine.SEQUENCE              AS PO_LINE,
        po.VENDOR_CODE, po.VENDOR_NAME,
        poLine.ITEM_CODE, poLine.ITEM_DESCRIPTION,
        poLine.PURCHASE_ITEM_UNIT_CODE AS PO_UNIT,
        costing.BL_NO, container.CONTAINER_NO, container.SITE_CODE,
        container.TICKET_NO, container.LORRY_PLATE_NO, container.DATE_IN
FROM    IMPORT_SCHEDULE_COSTING_SUB container
JOIN    IMPORT_SCHEDULE_COSTING costing ON costing.PKKEY = container.IMPORT_SCHEDULE_COSTING_ID
JOIN    PURCHASE_ORDER_SUB poLine       ON poLine.PKKEY  = costing.PURCHASE_ORDER_SUB_ID
JOIN    PURCHASE_ORDER po               ON po.REF_NO     = poLine.PURCHASE_ORDER_REF_NO
WHERE   container.REF_NO IS NOT NULL
AND     container.DATE_OUT IS NULL
```

进货的规则：
- **操作员选的是 Planning line / Costing container（Ref No），不是 PO line。** 一个 PO line 可以有很多个 Planning line（每车一个）。
- `REF_NO` 在两个表合起来是唯一的。格式是 `{PO No}-{Item Code}-{n}`，例如 `RM-TEST-01-RM-TEST-ITEM-1`，最长 120 字。不要自己拼，直接用 sync 过来的值。
- 已经有 `DATE_OUT` 的不能再选（已完成）。
- 有 `DATE_IN`、没有 `DATE_OUT` 的是“车在厂里”，等第二磅。
- `DELIVERY_DATE` 给操作员参考，API 不检查日期。

### 3.3 出货：可以选的 DO No

| 表 | 用途 |
|---|---|
| `DAILY_DELIVERY_PLANNING` | Planning 表头，`DELIVERY_DATE`、`DESPATCH_TICKET_REF_NO` |
| `DAILY_DELIVERY_PLANNING_SUB` | Planning line，`DESPATCH_TICKET_SUB_ID` 指向 ticket line，`TIME_IN` / `TIME_OUT` |
| `DESPATCH_TICKET_SUB` | Ticket line，`DO_NO`、客户、料号、重量 |
| `DESPATCH_TICKET` | Ticket 表头，车牌 `LORRY_PLATE_NO`、运输商 |
| `DESPATCH_TICKET_CONFIRMATION` | Confirm DO 的状态（`STATUS` = `Pending` / `Posted` / `Failed`） |

```sql
SELECT  ticketLine.DO_NO,
        ticket.REF_NO                AS DESPATCH_TICKET,
        ticket.LORRY_PLATE_NO,
        ticket.TRANSPORTER_CODE, ticket.TRANSPORTER_DESCRIPTION,
        planning.DELIVERY_DATE,
        ticketLine.CUSTOMER_CODE, ticketLine.CUSTOMER_NAME,
        ticketLine.ITEM_CODE, ticketLine.ITEM_DESCRIPTION,
        ticketLine.WEIGHT, ticketLine.ITEM_UNIT_CODE,
        ticketLine.DO_QUANTITY, ticketLine.DO_UNIT_CODE,
        planningLine.SITE_CODE,
        planningLine.TIME_IN, planningLine.TIME_OUT
FROM    DAILY_DELIVERY_PLANNING_SUB planningLine
JOIN    DAILY_DELIVERY_PLANNING planning ON planning.PKKEY = planningLine.DAILY_DELIVERY_PLANNING_ID
JOIN    DESPATCH_TICKET_SUB ticketLine   ON ticketLine.PKKEY = planningLine.DESPATCH_TICKET_SUB_ID
JOIN    DESPATCH_TICKET ticket           ON ticket.REF_NO = ticketLine.DESPATCH_TICKET_REF_NO
LEFT JOIN DESPATCH_TICKET_CONFIRMATION confirmation ON confirmation.DESPATCH_TICKET_REF_NO = ticket.REF_NO
WHERE   ticketLine.DO_NO IS NOT NULL
AND     CAST(planning.DELIVERY_DATE AS date) = CAST(GETDATE() AS date)
AND     planningLine.TIME_OUT IS NULL
AND     (confirmation.STATUS IS NULL OR confirmation.STATUS <> 'Posted')
```

出货的规则：
- **只有已经建立 Daily Delivery Planning 的 Despatch Ticket 才能过磅。** 只有 Despatch Ticket、没有 Planning 的 DO 会被 API 拒绝。
- **只允许 Delivery Date = 今天的 DO 过磅。** 这个检查由 Weighing System 负责，API 不检查。
- 同一个 DO No 可能出现在几个 line 上。API 按 DO No 一次更新所有这些 line，所以列表请按 `DO_NO` 合并成一行显示。
- 一张 Despatch Ticket 可能有几个不同的 DO No，每个 DO No 要各自过磅（各自 call Time In / Time Out）。
- Confirm DO 已经是 `Posted` 的 ticket 不能再改（API 会拒绝），不要列出来。

---

## 4. API

### 4.1 通用

- 测试环境 Base URL：`[测试网址]`；正式环境 Base URL：`[正式网址]`
- 认证：**HTTP Basic**，账号密码由我们提供。这个账号需要有 API 权限。
- 全部是 `POST`，Body 是 JSON，Header `Content-Type: application/json`。
- 时间格式一律是 **`yyyy-MM-dd HH:mm:ss`**（24 小时，要有秒），例如 `2026-10-27 08:05:00`。少了秒数会被拒绝。
- 数量 > 0，最多 3 位小数。用 JSON number 送，例如 `30.12`。
- 回复**永远是 HTTP 200**，body 是：

  ```json
  { "isSuccess": true, "message": "..." }
  ```

  请只看 `isSuccess`，不要看 HTTP status。`isSuccess` 是 `false` 时，`message` 是英文的原因，请原样显示给操作员。
- 认证失败也是 HTTP 200，`{ "isSuccess": false, "message": "Invalid username or password." }`。
- **可以重送**：在完成之前（见每个 API 的条件），同一个请求再送一次会覆盖之前的值，可以用来修正时间、车牌、数量。
- 网络超时、没有收到回复时：直接重送同一个请求即可，重送是安全的。

### 4.2 第一磅（进货）：`POST /api/ticketin`

```json
{
    "ref_no": "RM-TEST-01-RM-TEST-ITEM-1",
    "ticket_no": "RT-TEST-01",
    "lorry_no": "WTEST1",
    "date_in": "2026-10-27 08:05:00"
}
```

| 字段 | 必填 | 规则 |
|---|---|---|
| `ref_no` | ✔ | 操作员选的 Planning line / Costing container 的 `REF_NO` |
| `ticket_no` | ✔ | Weighing System 的过磅单号，最长 50 字。**一个 Ticket No 只能用在一个 line / container 上** |
| `lorry_no` | ✔ | 车牌，最长 20 字 |
| `date_in` | ✔ | 第一磅时间 |

条件：
- `ref_no` 找不到 → 失败 `Ref No ... is not in Raw Material Planning or Import Schedule & Costing.`
- 这个 line / container 已经 Ticket Out → 失败 `The ... is already ticketed out with Ticket No ....`
- `ticket_no` 已经用在别的 line / container → 失败 `Ticket No ... is already on another Raw Material Planning line or Import Schedule & Costing container.`
- 还没 Ticket Out 之前可以重送：覆盖车牌、时间，也可以换另一个 Ticket No（message 会写 `It replaces Ticket No ...`）。

成功：`Ticket In of Ticket No RT-TEST-01 is saved to Raw Material Planning line RM-TEST-01-RM-TEST-ITEM-1.`

### 4.3 第二磅（进货）：`POST /api/ticketout`

```json
{
    "ticket_no": "RT-TEST-01",
    "date_out": "2026-10-27 09:30:00",
    "received_qty": 30.12,
    "weighbridge_qty": 30.08,
    "supplier_delivery_qty": 30,
    "bin_location": "BIN-TEST",
    "lot_no": "LOT-TEST-01"
}
```

| 字段 | 必填 | 规则 |
|---|---|---|
| `ticket_no` | ✔ | 和 Ticket In 同一个 Ticket No（用它找回那个 line / container，不用再送 Ref No） |
| `date_out` | ✔ | 第二磅时间，不能早于 Ticket In 的 `date_in` |
| `received_qty` | ✔ | Received / Theoretical Qty，> 0，最多 3 位小数 |
| `weighbridge_qty` | ✔ | 由过磅重量算出来的数量，> 0，最多 3 位小数 |
| `supplier_delivery_qty` | ✔ | 供应商送货单上的数量，> 0，最多 3 位小数 |
| `bin_location` |  | 最长 15 字 |
| `lot_no` |  | 最长 20 字。不送的话，GRN 用出厂日期 `yyyyMMdd` |

**单位：三个数量都必须是 PO line 的单位**（sync 资料里的 `PO_UNIT` = `PURCHASE_ORDER_SUB.PURCHASE_ITEM_UNIT_CODE`）。例如 PO 单位是 `MT`，磅出来 30,080 kg，就送 `30.08`；PO 单位是 `KG` 就送 `30080`。网站不换算，也不收单位。

条件：
- 找不到这个 Ticket No（没有 Ticket In）→ 失败 `Ticket No ... is not in Raw Material Planning or Import Schedule & Costing.`
- `date_out` 早于 Time In → 失败 `Ticket No ...: date_out cannot be earlier than the Time In ....`
- 这张 Ticket 的 GRN 已经成功送到 CSL（`Posted`）→ 失败 `The GRN of Ticket No ... is already posted to CSL, so its Ticket Out can no longer be changed.`
- GRN 还没 `Posted` 之前可以重送，覆盖数量和时间。

**网站之后做的事（GRN 给 CSL）：**
1. 网站先保存 Ticket Out，然后马上 call CSL `Create_GRN` 一次。
2. 只要 Ticket Out 存好了，回复就是 `isSuccess: true`。`message` 的后半段写 GRN 的结果：
   - 成功：`Ticket Out of Ticket No ... is saved to .... <CSL 的结果>`
   - 失败：`Ticket Out of Ticket No ... is saved to .... Posting the GRN to CSL failed: <原因>`
3. GRN 失败**不用 Weighing System 处理，也不用重送 Ticket Out**。网站的用户会在 Planning / Costing 页面上按 Repost GRN。
4. Import Schedule & Costing container 如果还没有 Date Completed，网站会用 `date_out` 的日期填上。

### 4.4 第一磅（出货）：`POST /api/dotimein`

```json
{
    "do_no": "DO-TEST-01",
    "date_in": "2026-10-27 08:00:00"
}
```

| 字段 | 必填 | 规则 |
|---|---|---|
| `do_no` | ✔ | 操作员选的 DO No。所有带这个 DO No 的 planning line 都会写入 Time In |
| `date_in` | ✔ | 第一磅时间，不能晚于已有的 Time Out |

条件：
- DO No 不在任何 Despatch Ticket → 失败 `DO No ... is not in any Despatch Ticket.`
- 有 Despatch Ticket 但还没有 Daily Delivery Planning → 失败 `DO No ... is in Despatch Ticket ..., but its Daily Delivery Planning is not created yet.`
- 这张 ticket 的 Confirm DO 已经 `Posted` → 失败 `The delivery of Despatch Ticket ... is already confirmed to CSL, so the Time In of DO No ... can no longer be changed.`
- 可以重送，覆盖 Time In。

网站同时更新 Lorry Que（排队画面）：那辆车标为已到达，并记录进厂时间。

成功：`Time In of DO No DO-TEST-01 is saved (2 lines). Lorry Que ...`

### 4.5 第二磅（出货）：`POST /api/dotimeout`

```json
{
    "do_no": "DO-TEST-01",
    "date_out": "2026-10-27 09:30:00",
    "actual_weight": 25040,
    "actual_unit": "KG"
}
```

| 字段 | 必填 | 规则 |
|---|---|---|
| `do_no` | ✔ | 和 Time In 同一个 DO No |
| `date_out` | ✔ | 第二磅时间。必须先有 Time In，而且不能早于 Time In |
| `actual_weight` | ✔ | 实际重量，> 0，最多 3 位小数。会写到所有带这个 DO No 的 line |
| `actual_unit` | ✔ | `actual_weight` 的单位，必须是网站 Unit master 里有的代码，例如 `KG`、`MT` |

条件：
- 还没 Time In → 失败 `DO No ... has no Time In yet.`
- `date_out` 早于 Time In → 失败 `date_out cannot be earlier than the Time In ....`
- `actual_unit` 不在 Unit master → 失败 `actual_unit '...' is not in the Unit master.`
- Confirm DO 已经 `Posted` → 失败（同 4.4）。
- Confirm DO 还没 `Posted` 之前可以重送，覆盖时间和重量。

**网站之后做的事（Confirm DO 给 CSL）：**
1. 当这次 Time Out 让这张 Despatch Ticket 的**所有** planning line 都有 Time Out 时（即最后一个 DO 出厂），网站 call CSL `Confirm_DO` 一次。
2. 回复一样是 `isSuccess: true`，`message` 后半段写 Confirm DO 的结果，例如 `... Confirming the delivery of Despatch Ticket ... to CSL failed: <原因>`。
3. 失败**不用 Weighing System 处理**，网站用户会在 Daily Delivery Planning 页面按 Repost。
4. 成功（`Posted`）以后，这张 ticket 的所有 DO 都不能再 Time In / Time Out。

### 4.6 失败的格式错误（四个 API 共用）

| 情况 | message |
|---|---|
| 缺必填字段 | `ref_no, lorry_no, date_in is required.`（列出缺的字段） |
| 时间格式错 | `date_in '2026-10-27 08:05' is not a valid date and time. Use yyyy-MM-dd HH:mm:ss.` |
| 数量不对（≤ 0、超过 3 位小数、不是数字） | `received_qty '30.1234' must be a number greater than 0 with at most 3 decimals.` |
| 太长 | `ticket_no cannot be longer than 50 characters.` |
| 账号密码错 | `Invalid username or password.` |

---

## 5. 在测试环境自己做测试资料（重要）

测试资料不用等我们准备，你可以在测试网站上自己做，随时做新的。全部在浏览器操作，不用碰数据库。

**准备：**
- 测试网站：`[测试网址]`，登入账号：`[账号]` / `[密码]`（我们提供）。
- 下面会用到的菜单：**API Demo**、**Receiving → Purchase Order / Raw Material Planning / Import Schedule & Costing**、**Despatch → Despatch Ticket / Daily Delivery Planning / Lorry Que**。
- **API Demo** 页面是测试用的，它可以模拟 CSL 把 PO / Despatch Ticket post 到网站，也可以模拟 Weighing System call 四个 API（用来对照你们的实现）。

**重要：每一轮测试都做新的资料**（新的 PO No、新的 Despatch Ticket No）。Ticket Out / DO Time Out 完成、GRN / Confirm DO 成功以后，那笔资料就不能再用了。建议编号加上自己的前缀和流水号，例如 `WB-PO-0001`、`WB-DT-0001`，避免和别人的测试资料混在一起。

### 5.1 进货资料：PO → Raw Material Planning

1. **模拟 CSL 发 PO**：打开 **API Demo** → **Purchase Order API (CSL)** 卡片。
   - **PO No (po_number)**：填新的编号，例如 `WB-PO-0001`。
   - 按 **Select First**：Site、Division、Supplier、Currency、Terms 自动填上第一笔 master；line 表格会有 Item。
   - Line 表格：每个 line 选 **Item**、填 **Qty**。**Unit** 留空就是 Item 本身的单位；想测试换算，就选一个不同的单位（例如 `MT`）。
   - 按 **Send**。下面的 Result 显示 request / response，成功就是 PO 已经进网站了。
2. **建立 Planning**：打开 **Receiving → Purchase Order**，找到刚才的 PO line（Status 是 Open），按该行右边的 **⋮** → **Create Raw Material Planning**。
3. 在 Planning 页面：
   - **Period Type** 留空（Specific Date）时，**Date** 必填，填今天。
   - **Delivery Details** 表格按 **Add** 加几行（一行 = 一车）：填 **Delivery Qty**、**Location**（Site）。
   - 按 **Save**。保存时每一行会自动生成 Ref No。
4. **看 Ref No**：Ref No 不显示在 Planning 页面上。可以在 **API Demo → Ticket In API (Weighbridge)** 的列表看到 **Ref No (ref_no)** 栏，或在 sync 过来的 `RAW_MATERIAL_PLANNING_SUB.REF_NO` 看到。
5. 资料 sync 到 Weighing System 后，就可以选这些 line 来测试 Ticket In / Ticket Out。

### 5.2 进货资料：PO → Import Schedule & Costing（进口货柜）

1. 按 5.1 第 1 步再发一个 PO（或用同一个 PO 的另一个 line）。
2. **Receiving → Purchase Order**，PO line 右边 **⋮** → **Create Import Schedule & Costing**。
3. 在 Costing 页面的 **Container Details** 表格按 **Add** 加 container：填 **Container No**、**Site**。
4. 按 **Save**。每个 container 自动生成 Ref No（同样在 API Demo 的 Ticket In 列表或 `IMPORT_SCHEDULE_COSTING_SUB.REF_NO` 看到）。

注意：一个 PO line 只能有一个 Raw Material Planning 和一个 Import Schedule & Costing。PO line 的 ⋮ 已经变成 **Edit ...** 的话，表示已经建过了，可以进去加 line / container，或发新的 PO。

### 5.3 出货资料：Despatch Ticket → Daily Delivery Planning

1. **模拟 CSL 发 Despatch Ticket**：打开 **API Demo** → **Despatch Ticket API (CSL)** 卡片。
   - **Ticket (cs_ticket)**：填新的编号，例如 `WB-DT-0001`。
   - 按 **Select First**：Site、Division、Lorry、Transporter、Vendor、Weight Unit 自动填上。
   - Line 表格：每个 line 选 **Customer**、**Item**，填 **Weight**。**DO No** 可以自己填（例如 `WB-DO-0001`）；留空的话自动用 `DO-{cs_ticket}-{line}`，例如 `DO-WB-DT-0001-1`。想测试“一张 ticket 多个 DO”，就给不同 line 填不同 DO No；想测试“一个 DO 多个 line”，就给几个 line 填同一个 DO No。
   - 按 **Send**。
2. **建立 Daily Delivery Planning**：打开 **Despatch → Despatch Ticket**，找到刚才的 ticket，按右边 **⋮** → **Create Daily Delivery Planning**。
3. 在 Planning 页面：**Delivery Date 填今天**（不填的话 Weighing System 按规则不能选它）。每个 line 的 **Site** 可以改。按 **Save**。
4. 资料 sync 到 Weighing System 后，就可以选这个 DO No 来测试 DO Time In / DO Time Out。
5. 结果可以在 **Despatch → Daily Delivery Planning**（line 的 Time In / Time Out / Actual Quantity）和 **Despatch → Lorry Que**（今天的车）看到。

### 5.4 检查结果

| 测试了 | 去网站哪里看 |
|---|---|
| Ticket In / Ticket Out（Planning） | **Receiving → Raw Material Planning**，打开那张 Planning，line 的 Lorry、Ticket No、Time In、Time Out、Received / Weighbridge / Supplier Delivery Qty |
| Ticket In / Ticket Out（Costing） | API Demo 的 Ticket In / Ticket Out 列表（Ticket No、Time In）；container 的 Date In / Date Out |
| DO Time In / Time Out | **Despatch → Daily Delivery Planning**（line 的 Time In / Time Out / Actual Quantity / Actual Unit）、**Despatch → Lorry Que** |
| 网站有没有收到你们的 request | 每次回复的 `message` |

测试环境的 GRN / Confirm DO 也会按测试环境的设定 call CSL。测试资料在 CSL 那边不存在，所以 `message` 后半段写 `Posting the GRN to CSL failed: ...` 或 `Confirming the delivery ... failed: ...` 是正常的，不影响 Weighing System 的测试。也因为是 `Failed`，同一笔资料还可以继续重送测试。

### 5.5 用 Postman 测

1. 导入 `AllcosmosSystem - Weighbridge API (Normal Cases).postman_collection.json`。
2. Collection → **Variables**：`baseUrl` 填测试网址，`username` / `password` 填我们给的账号。
3. 把 body 里的 `ref_no`、`ticket_no`、`do_no`、时间换成你在第 5.1–5.3 节做的资料，按顺序送：1 Ticket In → 2 Ticket Out → 3 DO Time In → 4 DO Time Out。
4. 每个 request 的 Description 写着字段规则和预期的回复。

### 5.6 建议的测试项目

进货：
1. 正常：Ticket In → Ticket Out，Planning 页面看到所有值。
2. Ticket In 重送（改车牌 / 时间）→ 覆盖。
3. Ticket In 换另一个 Ticket No → message 写 `It replaces Ticket No ...`。
4. 同一个 Ticket No 用在第二个 line → 失败。
5. 没有 Ticket In 就 Ticket Out → 失败。
6. `date_out` 早于 `date_in` → 失败。
7. 已经 Ticket Out 的 line 再 Ticket In → 失败。
8. PO 单位是 `MT` 和 `KG` 各测一次，确认你们送的数量已经换算成 PO 单位。
9. Costing container 走一次 Ticket In → Ticket Out。

出货：
1. 正常：DO Time In → DO Time Out，Daily Delivery Planning 和 Lorry Que 看到值。
2. 一张 ticket 两个 DO：第一个 DO Time Out 后不会 Confirm DO；第二个 DO Time Out 后 message 才写 Confirm DO 的结果。
3. 没有 Time In 就 Time Out → 失败。
4. `actual_unit` 乱填（例如 `XYZ`）→ 失败。
5. 只有 Despatch Ticket、还没建 Daily Delivery Planning 的 DO → 失败。

---

## 6. 待确认

- 测试 / 正式环境的网址、API 账号：由我们提供。
- 资料 sync 的方式和频率：[待定]。
