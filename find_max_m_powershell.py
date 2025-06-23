import subprocess
import time

MAX_M = 100

def run(cmd):
    """Run a command and return its output as a string."""
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

def start_vote_process(i, m):
    """Launch avpvote.exe with arguments i 0 m using PowerShell Start-Process."""
    ps_cmd = f'Start-Process -NoNewWindow avpvote.exe -ArgumentList "{i} 0 {m}"'
    subprocess.run(["powershell", "-Command", ps_cmd])

def kill_avpvote_processes():
    subprocess.run(
        ["powershell", "-Command", "Stop-Process -Name avpvote -Force -ErrorAction SilentlyContinue"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

def main():
    for m in range(1, MAX_M + 1):
        print(f"\n🔍 Testing with m = {m} parties...")
        kill_avpvote_processes()
        run("avpclean.exe")
        run("avpinit.exe")

        for i in range(m):
            start_vote_process(i, m)

        # Wait for votes to finish
        time.sleep(30)

        # Run tally and provide m via stdin
        proc = subprocess.run("avptally.exe", input=f"{m}\n", text=True,
                              capture_output=True, shell=True)
        output = proc.stdout.strip()
        print(output)

        if "ALL VOTED YES" not in output:
            print(f"\n❌ Protocol broke at m = {m}. ✅ Max safe m = {m - 1}")
            break
    else:
        print(f"\n✅ SUCCESS: All m={MAX_M} parties passed.")

if __name__ == "__main__":
    main()



# import subprocess
# import time
# import os

# MAX_M = 100

# def run(cmd):
#     """Run a command and return its output as a string."""
#     result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
#     return result.stdout.strip()

# def run_powershell_vote(i, m):
#     """Use PowerShell Start-Process to spawn avpvote.exe i 0 m."""
#     ps_cmd = f'Start-Process avpvote.exe -ArgumentList "{i} 0 {m}"'
#     subprocess.run(["powershell", "-Command", ps_cmd])

# def wait_for_all_votes(m, timeout=10):
#     expected = [f"PartyVote_{i}" for i in range(m)]
#     start = time.time()

#     while time.time() - start < timeout:
#         existing = [name for name in os.listdir(".") if name in expected]
#         if len(existing) == m:
#             return True
#         time.sleep(0.2)

#     print("⚠️ Timeout: Not all PartyVote_i files appeared.")
#     return False

# def main():
#     for m in range(1, MAX_M + 1):
#         print(f"\n🔍 Testing with m = {m} parties...")

#         run("avpclean.exe")
#         run("avpinit.exe")

#         for i in range(m):
#             run_powershell_vote(i, m)

#         # ✅ Wait for all votes to finish before tallying
#         if not wait_for_all_votes(m):
#             print(f"\n❌ Vote writing failed at m = {m}. Aborting...")
#             break

#         proc = subprocess.run("avptally.exe", input=f"{m}\n", text=True,
#                               capture_output=True, shell=True)
#         output = proc.stdout.strip()
#         print(output)

#         if "ALL VOTED YES" not in output:
#             print(f"\n❌ Someone vetoed at m = {m}. ✅ Max m = {m-1}")
#             break
#     else:
#         print(f"\n✅ SUCCESS: All m={MAX_M} parties voted YES correctly.")

# if __name__ == "__main__":
#     main()
