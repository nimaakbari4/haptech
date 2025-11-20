import trials

SUBJECT = "landon"
TRIAL_PREFIX = "example_experiment_trial_"
NUM_EXAMPLE_TRIALS = 2
NUM_BALL_TRIALS = 2

def main():
    for i in range(NUM_EXAMPLE_TRIALS):
        one_trial(f"trial_example.xml", SUBJECT, f"{TRIAL_PREFIX}A{i + 1}")

    for i in range(NUM_BALL_TRIALS):
        one_trial(f"trial_example_2.xml", SUBJECT, f"{TRIAL_PREFIX}B{i + 1}")
    
def one_trial(trial_config, subject, id):
    trials.run_trial("home_robot.xml", subject, id)
    trials.run_trial(trial_config, subject, id)

if __name__ == "__main__":
    main()