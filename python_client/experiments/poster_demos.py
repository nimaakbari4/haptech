import trials

SUBJECT = "poster_demos"
TRIAL_PREFIX = "demo_"

def main():
    while True:
        input("Press Enter to run Pong game... (CTRL+C to exit)")
        trials.run_trial(f"pong_demo.xml", SUBJECT, f"{TRIAL_PREFIX}_pong")
        input("Press Enter to run Sandbox.. (CTRL+C to exit)")
        trials.run_trial(f"sandbox.xml", SUBJECT, f"{TRIAL_PREFIX}_sandbox")

if __name__ == "__main__":
    main()