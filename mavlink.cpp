#include "mavlink.h"

#include <unistd.h>
#include "mavlink/include/mavlink/v1.0/common/mavlink.h"
#include "mavlink/include/mavlink/v1.0/mavlink_types.h"

#include <cstdlib>
#include <cmath>
#include <inttypes.h>
#include <fstream>
// Serial includes
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#ifdef __linux
#include <sys/ioctl.h>
#endif
// Latency Benchmarking
#include <sys/time.h>
#include <time.h>
#include "mavlink/include/mavlink/v1.0/common/common.h"
#include <ctime>

using std::string;
using namespace std;

#define MAVLINK_OFFBOARD_CONTROL_MODE_NONE 0
#define MAVLINK_OFFBOARD_CONTROL_MODE_RATES 1
#define MAVLINK_OFFBOARD_CONTROL_MODE_ATTITUDE 2
#define MAVLINK_OFFBOARD_CONTROL_MODE_VELOCITY 3
#define MAVLINK_OFFBOARD_CONTROL_MODE_POSITION 4
#define MAVLINK_OFFBOARD_CONTROL_FLAG_ARMED 0x10
#ifndef INT16_MAX
#define INT16_MAX 0x7fff
#endif
#ifndef INT16_MIN
#define INT16_MIN (-INT16_MAX - 1)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#endif

int g_Compass = 0;
int g_Head = 0;

struct timeval tv;		  ///< System time
int serial_compid = 0;
bool silent = false;              ///< Wether console output should be enabled
bool verbose = false;             ///< Enable verbose output
bool debug = false;               ///< Enable debug functions and output
int fd;

// Settings
//int sysid = 42;             ///< The unique system id of this MAV, 0-127. Has to be consistent across the system
int sysid = 1;             ///< The unique system id of this MAV, 0-127. Has to be consistent across the system

//int compid = 110;
int compid = MAV_COMP_ID_IMU;

/////////////////----------------------
mavlink_system_t mavlink_system;
 
 
// Define the system type, in this case an airplane
uint8_t system_type = MAV_TYPE_FIXED_WING;
uint8_t autopilot_type = MAV_AUTOPILOT_GENERIC;

uint8_t system_mode = MAV_MODE_PREFLIGHT; ///< Booting up
uint32_t custom_mode = 0;                 ///< Custom mode, can be defined by user/adopter
uint8_t system_state = MAV_STATE_STANDBY; ///< System ready for flight
 
// Initialize the required buffers
mavlink_message_t msgg;
uint8_t bufg[MAVLINK_MAX_PACKET_LEN];
/////////////////----------------------
int open_port(const char* port)
{
	int fd; /* File descriptor for the port */
	
	// Open serial port
	// O_RDWR - Read and write
	// O_NOCTTY - Ignore special chars like CTRL-C
	fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		/* Could not open the port. */
		return(-1);
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
	}
	
	return (fd);
}

bool setup_port(int fd, int baud, int data_bits, int stop_bits, bool parity, bool hardware_control)
{
	//struct termios options;
	
	struct termios  config;
	if(!isatty(fd))
	{
		fprintf(stderr, "\nERROR: file descriptor %d is NOT a serial port\n", fd);
		return false;
	}
	if(tcgetattr(fd, &config) < 0)
	{
		fprintf(stderr, "\nERROR: could not read configuration of fd %d\n", fd);
		return false;
	}
	//
	// Input flags - Turn off input processing
	// convert break to null byte, no CR to NL translation,
	// no NL to CR translation, don't mark parity errors or breaks
	// no input parity check, don't strip high bit off,
	// no XON/XOFF software flow control
	//
	config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
	                    INLCR | PARMRK | INPCK | ISTRIP | IXON);
	//
	// Output flags - Turn off output processing
	// no CR to NL translation, no NL to CR-NL translation,
	// no NL to CR translation, no column 0 CR suppression,
	// no Ctrl-D suppression, no fill characters, no case mapping,
	// no local output processing
	//
	//config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
	  //                   ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
	config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
	                     ONOCR | OFILL | OLCUC | OPOST);

	//
	// No line processing:
	// echo off, echo newline off, canonical mode off,
	// extended input processing off, signal chars off
	//
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	//
	// Turn off character processing
	// clear current char size mask, no parity checking,
	// no output processing, force 8 bit input
	//
	config.c_cflag &= ~(CSIZE | PARENB);
	config.c_cflag |= CS8;
	//
	// One input byte is enough to return from read()
	// Inter-character timer off
	//
	config.c_cc[VMIN]  = 1;
	config.c_cc[VTIME] = 10; // was 0
	
	// Get the current options for the port
	//tcgetattr(fd, &options);
	
	switch (baud)
	{
		case 1200:
			if (cfsetispeed(&config, B1200) < 0 || cfsetospeed(&config, B1200) < 0)
			{
				fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
				return false;
			}
			break;
		case 1800:
			cfsetispeed(&config, B1800);
			cfsetospeed(&config, B1800);
			break;
		case 9600:
			cfsetispeed(&config, B9600);
			cfsetospeed(&config, B9600);
			break;
		case 19200:
			cfsetispeed(&config, B19200);
			cfsetospeed(&config, B19200);
			break;
		case 38400:
			if (cfsetispeed(&config, B38400) < 0 || cfsetospeed(&config, B38400) < 0)
			{
				fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
				return false;
			}
			break;
		case 57600:
			if (cfsetispeed(&config, B57600) < 0 || cfsetospeed(&config, B57600) < 0)
			{
				fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
				return false;
			}
			break;
		case 115200:
			if (cfsetispeed(&config, B115200) < 0 || cfsetospeed(&config, B115200) < 0)
			{
				fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
				return false;
			}
			break;

		// These two non-standard (by the 70'ties ) rates are fully supported on
		// current Debian and Mac OS versions (tested since 2010).
		case 460800:
			if (cfsetispeed(&config, 460800) < 0 || cfsetospeed(&config, 460800) < 0)
			{
				fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
				return false;
			}
			break;
		case 921600:
			if (cfsetispeed(&config, 921600) < 0 || cfsetospeed(&config, 921600) < 0)
			{
				fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
				return false;
			}
			break;
		default:
			fprintf(stderr, "ERROR: Desired baud rate %d could not be set, aborting.\n", baud);
			return false;
			
			break;
	}
	
	//
	// Finally, apply the configuration
	//
	if(tcsetattr(fd, TCSAFLUSH, &config) < 0)
	{
		fprintf(stderr, "\nERROR: could not set configuration of fd %d\n", fd);
		return false;
	}
	return true;
}

void close_port(int fd)
{
	close(fd);
}

int serial_wait(/*int serial_fd*/)
{

	
	// int fd = serial_fd;
	
	mavlink_status_t lastStatus;
	lastStatus.packet_rx_drop_count = 0;
	
	
	// mavlink_msg_heartbeat_pack(mavlink_system.sysid, mavlink_system.compid, &msgg, system_type, autopilot_type, system_mode, custom_mode, system_state);
	// uint16_t len = mavlink_msg_to_send_buffer(bufg, &msgg);
	// write(fd, bufg, len);
	
	// Blocking wait for new data
	while (1)
	{
		//if (debug) printf("Checking for new data on serial port\n");
		// Block until data is available, read only one byte to be able to continue immediately
		//char buf[MAVLINK_MAX_PACKET_LEN];
		uint8_t cp;
		mavlink_message_t message;
		mavlink_status_t status;
		uint8_t msgReceived = false;
		//tcflush(fd, TCIFLUSH);
		if (read(fd, &cp, 1) > 0)
		{
			// Check if a message could be decoded, return the message in case yes
			msgReceived = mavlink_parse_char(MAVLINK_COMM_1, cp, &message, &status);
			if (lastStatus.packet_rx_drop_count != status.packet_rx_drop_count)
			{
				if (verbose || debug) printf("ERROR: DROPPED %d PACKETS\n", status.packet_rx_drop_count);
				if (debug)
				{
					unsigned char v=cp;
					fprintf(stderr,"%02x ", v);
				}
			}
			lastStatus = status;
		}
		else
		{
			if (!silent) fprintf(stderr, "ERROR: Could not read from fd %d\n", fd);
		}
		
		// If a message could be decoded, handle it
		if(msgReceived)
		{
			//if (verbose || debug) std::cout << std::dec << "Received and forwarded serial port message with id " << static_cast<unsigned int>(message.msgid) << " from system " << static_cast<int>(message.sysid) << std::endl;
			
			// Do not send images over serial port
			
			// DEBUG output
			if (debug)
			{
				fprintf(stderr,"Received serial data: ");
				unsigned int i;
				uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
				unsigned int messageLength = mavlink_msg_to_send_buffer(buffer, &message);
				if (messageLength > MAVLINK_MAX_PACKET_LEN)
				{
					fprintf(stderr, "\nFATAL ERROR: MESSAGE LENGTH IS LARGER THAN BUFFER SIZE\n");
				}
				else
				{
					for (i=0; i<messageLength; i++)
					{
						unsigned char v=buffer[i];
						fprintf(stderr,"%02x ", v);
					}
					fprintf(stderr,"\n");
				}
			}
			
			if (verbose || debug)
				printf("Received message from serial with ID #%d (sys:%d|comp:%d):\n", message.msgid, message.sysid, message.compid);
			
			/* decode and print */


			// For full MAVLink message documentation, look at:
			// https://pixhawk.ethz.ch/mavlink/

			// Only print every n-th message
			static unsigned int scaled_imu_receive_counter = 0;
			switch (message.msgid)
			{
				// case MAVLINK_MSG_ID_RAW_IMU:
				// {
				// 	/*if (scaled_imu_receive_counter % 50 == 0)
				// 	{*/
						
						// mavlink_raw_imu_t imu;
						// mavlink_msg_raw_imu_decode(&message, &imu);
						// zgyro = imu.zgyro;
						// printf("Got message RAW_IMU\n");
						// printf("\t xacc: %d\n", imu.xacc);
						// printf("\t yacc: %d\n", imu.yacc);
						// printf("\t zacc: %d\n", imu.zacc);
						// printf("\t xgyro: %d\n", imu.xgyro);
						// printf("\t ygyro: %d\n", imu.ygyro);
						// printf("\t zgyro: %d\n", imu.zgyro);
						// printf("\t xmag: %d\n", imu.xmag);
						// printf("\t ymag: %d\n", imu.ymag);
						// printf("\t zmag: %d\n", imu.zmag);

				// // 		printf("Got message HIGHRES_IMU (spec: https://pixhawk.ethz.ch/mavlink/#HIGHRES_IMU)\n");
				// // 		printf("\t time: %llu\n", imu.time_usec);
				// // 		printf("\t acc  (NED):\t% f\t% f\t% f (m/s^2)\n", imu.xacc, imu.yacc, imu.zacc);
				// // 		printf("\t gyro (NED):\t% f\t% f\t% f (rad/s)\n", imu.xgyro, imu.ygyro, imu.zgyro);
				// // 		printf("\t mag  (NED):\t% f\t% f\t% f (Ga)\n", imu.xmag, imu.ymag, imu.zmag);
				// // 		printf("\t baro: \t %f (mBar)\n", imu.abs_pressure);
				// // 		printf("\t altitude: \t %f (m)\n", imu.pressure_alt);
				// // 		printf("\t temperature: \t %f C\n", imu.temperature);
				// // 		printf("\n");
					// }
				// // 	// scaled_imu_receive_counter++;
				// // }
				// // case MAVLINK_MSG_ID_SCALED_PRESSURE:
				// // {
				// // 	mavlink_scaled_pressure_t sp;
				// // 	mavlink_msg_scaled_pressure_decode(&message, &sp);

				// // 	printf("\t press_abs: %f\n", sp.press_abs);
				// // 	printf("\t press_diff: %f\n", sp.press_diff);
				// }
				case MAVLINK_MSG_ID_VFR_HUD:
				{
					// if (scaled_imu_receive_counter % 50 == 0)
					// {
						// printf("Got message VFR_HUD\n");
						mavlink_vfr_hud_t vfr;
						mavlink_msg_vfr_hud_decode(&message, & vfr);
						g_Compass = vfr.heading;
						return 0;
						// printf("\t heading: %d\n", vfr.heading);
						// printf("\t alt: %f\n", vfr.alt);
					// }
				// 	// scaled_imu_receive_counter++;
				}
				// // case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
				// // {
				// // 	mavlink_global_position_int_t gp;
				// // 	mavlink_msg_global_position_int_decode(&message, &gp);
				// // 	printf("\t vx %d\n", gp.vx);
				// // }
				// break;
				
			}
		}
	}
	return 0;
}

void InitMavlink()
{
	/* default values for arguments */
	char *uart_name = (char*)"/dev/ttySAC3";
	int baudrate = 57600;
	const char *commandline_usage = "\tusage: %s -d <devicename> -b <baudrate> [-v/--verbose] [--debug]\n\t\tdefault: -d %s -b %i\n";

	/* read program arguments */

	// Open the serial port.
	if (!silent) printf("Trying to connect to %s.. ", uart_name);
	fflush(stdout);

	fd = open_port(uart_name);
	if (fd == -1)
	{
		if (!silent) printf("failure, could not open port.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		if (!silent) printf("success.\n");
	}
	if (!silent) printf("Trying to configure %s.. ", uart_name);
	bool setup = setup_port(fd, baudrate, 8, 1, false, false);
	if (!setup)
	{
		if (!silent) printf("failure, could not configure port.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		if (!silent) printf("success.\n");
	}

	int noErrors = 0;
	if (fd == -1 || fd == 0)
	{
		if (!silent) fprintf(stderr, "Connection attempt to port %s with %d baud, 8N1 failed, exiting.\n", uart_name, baudrate);
		exit(EXIT_FAILURE);
	}
	else
	{
		if (!silent) fprintf(stderr, "\nConnected to %s with %d baud, 8 data bits, no parity, 1 stop bit (8N1)\n", uart_name, baudrate);
	}
	
	if(fd < 0)
	{
		exit(noErrors);
	}

	// Run indefinitely while the serial loop handles data
	if (!silent) printf("\nREADY, waiting for serial data.\n");

	mavlink_system.sysid = 1;                   ///< ID 20 for this airplane
	mavlink_system.compid = MAV_COMP_ID_IMU;     ///< The component sending the message is the IMU, it could be also a Linux process
	mavlink_system.type = MAV_TYPE_GCS;   ///< This system is an airplane / fixed wing

	mavlink_msg_heartbeat_pack(mavlink_system.sysid, mavlink_system.compid, &msgg, system_type, autopilot_type, system_mode, custom_mode, system_state);
	uint16_t len = mavlink_msg_to_send_buffer(bufg, &msgg);
	write(fd, bufg, len);
	// int head;
	while(g_Compass == 0)	serial_wait();
	g_Head = g_Compass;
	printf("Head: %d\n", g_Head);
}

void CloseMavlink()
{
	close_port(fd);
}