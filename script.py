import requests
import time
import sys
from urllib.parse import urlparse

# --- A reliable list of plain HTTP URLs for testing ---
URLS_TO_TEST = [
    # A classic small HTML page. Tests basic functionality.
    "http://info.cern.ch/hypertext/WWW/TheProject.html",
    # A medium-sized (~580KB) plain text file. Good for showing a clear difference.
    "http://www.textfiles.com/humor/devils.jok",
    # A very large (~6MB) plain text file. Will show a dramatic speed-up.
    "http://norvig.com/big.txt",
    # Another classic, small text file.
    "http://www.360doc.com/index.html"
]

# --- ANSI color codes for prettier output ---
class Colors:
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    RESET = '\033[0m'

def print_header(proxy_address):
    """Prints the benchmark header."""
    print("\n" + "="*95)
    print(f"  PROXY PERFORMANCE BENCHMARK | Testing against: {proxy_address}")
    print("="*95)
    print(f"{'URL Path':<40} | {'State':<15} | {'1st Hit (ms)':>12} | {'2nd Hit (ms)':>12} | {'Improvement':>12}")
    print("-"*95)

def test_url(url, proxy_address):
    """
    Tests a single URL by requesting it twice through the proxy.
    Returns a dictionary with the timing results.
    """
    proxies = {"http": proxy_address}
    results = {'url': url, 'time_miss': -1, 'time_hit': -1, 'error': None}

    try:
        # --- First Request (should be a Cache Miss) ---
        start_time_miss = time.perf_counter()
        requests.get(url, proxies=proxies, timeout=15)
        end_time_miss = time.perf_counter()
        results['time_miss'] = (end_time_miss - start_time_miss) * 1000

        # --- Second Request (should be a Cache Hit) ---
        start_time_hit = time.perf_counter()
        requests.get(url, proxies=proxies, timeout=15)
        end_time_hit = time.perf_counter()
        results['time_hit'] = (end_time_hit - start_time_hit) * 1000

    except requests.exceptions.RequestException as e:
        results['error'] = str(e)

    return results

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python proxy_benchmark.py <port>")
        print("Example: python proxy_benchmark.py 8080")
        sys.exit(1)

    port = sys.argv[1]
    proxy_server_address = f"http://127.0.0.1:{port}"

    print_header(proxy_server_address)

    for url in URLS_TO_TEST:
        result = test_url(url, proxy_server_address)

        # --- Format the output for the results table ---
        display_url = urlparse(url).path
        if len(display_url) > 38:
            display_url = "..." + display_url[-35:]
        
        if result['error']:
            print(f"{display_url:<40} | {Colors.RED}{'ERROR':<15}{Colors.RESET} | {result['error']}")
            continue

        time_miss_ms = result['time_miss']
        time_hit_ms = result['time_hit']
        
        improvement_str = ""
        cache_state = f"{Colors.YELLOW}MISS -> MISS{Colors.RESET}"

        if time_hit_ms < time_miss_ms and time_miss_ms > 0:
            improvement_val = ((time_miss_ms - time_hit_ms) / time_miss_ms) * 100
            if improvement_val > 5: # Only show significant improvements
                improvement_str = f"{Colors.GREEN}{improvement_val:.2f}%{Colors.RESET}"
                cache_state = f"{Colors.GREEN}MISS -> HIT{Colors.RESET}"

        print(f"{display_url:<40} | {cache_state:<25} | {time_miss_ms:>12.2f} | {time_hit_ms:>12.2f} | {improvement_str:>18}")
    
    print("-"*95)
    print("Benchmark complete.\n")