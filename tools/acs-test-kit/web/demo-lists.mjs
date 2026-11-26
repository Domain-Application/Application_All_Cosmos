import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  await page.goto('https://allcosmos.domainsb.com.my/APIDemo/Index');
  await page.waitForTimeout(6000);
  return await mainWorld(page, `
    const get = id => (document.getElementById(id)?.getDataList() ?? []).filter(r => /WB-/.test(JSON.stringify(r)));
    return { ticketIn: get('api-demo-ticket-in-table'), ticketOut: get('api-demo-ticket-out-table'), doIn: get('api-demo-do-time-in-table'), doOut: get('api-demo-do-time-out-table') };`);
}
