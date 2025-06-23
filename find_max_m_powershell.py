import subprocess  # Used to run shell commands and other executables
import time        # For delays and timing between processes

MAX_M = 100  # Maximum number of parties to test in the protocol

# Runs a shell command and returns its output as string
def run(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

# Starts a single vote process with party index `i` and total parties `m` using PowerShell
def start_vote_process(i, m):
    ps_cmd = f'Start-Process -NoNewWindow avpvote.exe -ArgumentList "{i} 0 {m}"'
    subprocess.run(["powershell", "-Command", ps_cmd])

# Force-kills any leftover avpvote.exe processes to ensure clean re-runs
def kill_avpvote_processes():
    subprocess.run(
        ["powershell", "-Command", "Stop-Process -Name avpvote -Force -ErrorAction SilentlyContinue"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

# Main loop to test increasing values of m from 1 to MAX_M
def main():
    for m in range(1, MAX_M + 1):
        print(f"\n🔍 Testing with m = {m} parties...")

        kill_avpvote_processes()  # Clean any previous hanging vote processes
        run("avpclean.exe")
        run("avpinit.exe")  

        # Launch m vote processes
        for i in range(m):
            start_vote_process(i, m)

        time.sleep(30)  # Wait for all vote processes to write their results

        # Run the tally process and pass the number of parties via input
        proc = subprocess.run("avptally.exe", input=f"{m}\n", text=True,
                              capture_output=True, shell=True)
        output = proc.stdout.strip()
        print(output)

        # If protocol breaks (i.e., output isn't "ALL VOTED YES"), stop testing
        if "ALL VOTED YES" not in output:
            print(f"\n❌ Protocol broke at m = {m}. ✅ Max safe m = {m - 1}")
            break
    else:
        print(f"\n✅ SUCCESS: All m={MAX_M} parties passed.")

if __name__ == "__main__":
    main() 
