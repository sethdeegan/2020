#include "main.h"

void tray_control(void*) {
	pros::Controller master(CONTROLLER_MASTER);
	pros::Task tray_t(tray_pid);
	bool b_toggle = false;
	while (true) {
		if (master.get_digital(DIGITAL_Y)) {
			b_toggle = !b_toggle;

			if (b_toggle) {
				for(int i=0;i<1700;i=i+3) {
					set_tray_pid(i);
					pros::delay(5);
				}
			} else {
				set_tray_pid(0);
			}

			while (master.get_digital(DIGITAL_Y)) {
				pros::delay(1);
			}
		}

		pros::delay(20);
	}
}

void arm_control(void*) {
	pros::Controller master(CONTROLLER_MASTER);
	pros::Task arm_t(arm_pid);
	bool was_pid;
	while (true) {
		if (master.get_digital(DIGITAL_B)) {
			was_pid = true;
			arm_t.resume();
			set_arm_pid(2300);
		} else if (master.get_digital(DIGITAL_DOWN)) {
			was_pid = true;
			arm_t.resume();
			set_arm_pid(1800);
		} else {
			if (master.get_digital(DIGITAL_R1)||master.get_digital(DIGITAL_R2)) {
				was_pid = false;
				set_arm((master.get_digital(DIGITAL_R1)-master.get_digital(DIGITAL_R2))*127);
			} else {
				if (!was_pid) {
					set_arm(0);
				}
			}
		}

		if (!was_pid) {
			arm_t.suspend();
		}

		pros::delay(20);
	}
}

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	auto chassis = ChassisControllerBuilder()
		.withMotors(
			{-1, -2}, // Left motors are 1 & 2 (reversed)
			{3, 4}    // Right motors are 3 & 4
		)
    // Green gearset, 4 in wheel diam, 11.5 in wheel track
    .withDimensions(AbstractMotor::gearset::green, {{4_in, 11.5_in}, imev5GreenTPR})
		.build();
		chassis->moveDistance(1_m);
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	pros::Controller master(CONTROLLER_MASTER);
	pros::Task tray_control_t(tray_control);

	pros::Task t(arm_control);
	while (true) {
		set_tank(master.get_analog(ANALOG_LEFT_Y), master.get_analog(ANALOG_RIGHT_Y));

		//set_arm((master.get_digital(DIGITAL_R1)-master.get_digital(DIGITAL_R2))*127);

		if (master.get_digital(DIGITAL_L1)) {
			set_rollers(127);
		} else if (master.get_digital(DIGITAL_L2)) {
			set_rollers(-127);
		} else if (master.get_digital(DIGITAL_RIGHT)) {
			set_rollers(60);
		} else {
			set_rollers(0);
		}

		pros::delay(20);
	}
}
