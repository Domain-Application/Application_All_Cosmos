export default async function run(page, ui) {
  await page.getByRole('button', { name: 'Open menu' }).click();
  await page.waitForTimeout(1000);
  const links = await page.evaluate(() => [...document.querySelectorAll('a')].map(a => (a.innerText.trim().replace(/\s+/g,' ') + ' -> ' + a.getAttribute('href'))).filter(s => s.length > 4));
  return links;
}
