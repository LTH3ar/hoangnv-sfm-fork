import time
import subprocess

def run_command(command, num_times):
    start_time = time.time()
    for _ in range(num_times):
        # Run your command here
        # You can use subprocess module to run external commands
        subprocess.run(command, shell=True)
    end_time = time.time()

    total_time = end_time - start_time

    hours = int(total_time // 3600)
    minutes = int((total_time % 3600) // 60)
    seconds = int(total_time % 60)
    milliseconds = int((total_time % 1) * 1000)

    return hours, minutes, seconds, milliseconds

# Example usage
command = "./app data/input/input.json data/input/map.txt"
num_times = 1000

hours, minutes, seconds, milliseconds = run_command(command, num_times)
print(f"Total time running: {hours}h {minutes}m {seconds}s {milliseconds}ms")