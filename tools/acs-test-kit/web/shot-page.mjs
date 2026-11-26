// Full-width screenshot of a website page after its data loads. Env: URL, OUT, TEXT (optional text to wait for).
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1920, height: 1080 });
  await page.goto(process.env.URL);
  if (process.env.TEXT) await page.getByText(process.env.TEXT).first().waitFor({ timeout: 20000 }).catch(() => {});
  await page.waitForTimeout(4000);
  await page.screenshot({ path: process.env.OUT });
  return (await page.evaluate(() => document.body.innerText)).replace(/\s+/g, ' ').slice(0, 1500);
}
