import sys
import argparse
import urllib.request
import json
from playwright.sync_api import sync_playwright

# 1. Parse arguments passed from C++ backend
parser = argparse.ArgumentParser(description="Creator Form Auto-Fill Automation Bridge")
parser.add_argument("--url", required=True, help="Target brand form URL")
parser.add_argument("--user", required=True, help="User UUID")
args = parser.parse_args()

print(f"[Python Automation] Starting Playwright bridge for User: {args.user}")

# 2. Fetch Creator Profile directly from Supabase REST API
# (Using your project credentials to fetch the vault data securely)
SUPABASE_URL = "https://aogqqciaztaxtjpeejhv.supabase.co"
SUPABASE_ANON_KEY = "sb_publishable_M6Pq1Ig8YDnDow6AkxIGYw_AyRrIuLG"

def fetch_creator_profile(user_id):
    try:
        api_url = f"{SUPABASE_URL}/rest/v1/creator_profiles?user_id=eq.{user_id}&select=instagram_handle,encrypted_shipping_address"
        req = urllib.request.Request(api_url, headers={
            "apikey": SUPABASE_ANON_KEY,
            "Authorization": f"Bearer {SUPABASE_ANON_KEY}"
        })
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            if data and len(data) > 0:
                return data[0]
    except Exception as e:
        print(f"[Supabase Fetch Error] {e}", file=sys.stderr)
    return None

profile = fetch_creator_profile(args.user)
if not profile:
    print(f"[Python Error] Profile not found in Supabase for user: {args.user}", file=sys.stderr)
    sys.exit(1)

instagram_handle = profile.get("instagram_handle", "")
shipping_address = profile.get("encrypted_shipping_address", "")

print(f"[Python Automation] Loaded Profile -> Instagram: {instagram_handle}, Address: {shipping_address}")

# 3. Execute Browser Automation via Playwright
try:
    with sync_playwright() as p:
        # Launch browser (headless=False so you can actually watch it work live!)
        browser = p.chromium.launch(headless=False)
        page = browser.new_page()
        
        print(f"[Playwright] Navigating to target URL: {args.url}")
        page.goto(args.url)
        
        # NOTE: Generic selector mapping. When targeting a specific brand form, 
        # you can update these CSS selectors (e.g., input[name='instagram'], textarea[name='address'])
        try:
            print("[Playwright] Attempting to fill form fields...")
            page.fill("input[name*='instagram'], input[id*='instagram']", instagram_handle)
            page.fill("textarea[name*='address'], input[id*='address']", shipping_address)
            print("[Playwright] Fields populated successfully!")
        except Exception as field_err:
            print(f"[Playwright Warning] Could not auto-detect standard selectors on this page: {field_err}")
            print("[Playwright] Page loaded successfully, keeping browser open for inspection.")

        # Keep browser open for a few seconds to let you see the result
        page.wait_for_timeout(5000)
        browser.close()

    sys.exit(0)

except Exception as e:
    print(f"[Python Automation Error] {e}", file=sys.stderr)
    sys.exit(1)