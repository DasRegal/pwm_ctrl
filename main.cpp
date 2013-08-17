#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "msp_pwm.h"
#include "cd74ac153.h"
#include "sonar.h"
#include <string.h>
#include "flight_routines.h"
#include "main.h"
#include "spi.h"

#include "find.h"
#include "define.h"

#include <curses.h>
#include <iostream>
#include <ctime>

// #include "mavlink/include/mavlink/v1.0/common/mavlink.h"
#include "mavlink/include/mavlink/v1.0/common/mavlink.h"
#include "mavlink/include/mavlink/v1.0/mavlink_types.h"

// Standard includes
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

// Дефайны
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

#define		SONAR1	0
#define		R_SONAR	1
#define		F_SONAR	2
#define 	SONAR4	3
#define 	SONAR5	4

 // 27 вывод
#define MISO 88
// 30 вывод
#define MOSI 114
// 18 вывод
#define CLK 115
// 34 вывод
#define CS1	105
// 31 вывод
#define CS2	87

int compass;
int zgyro;

int serial_wait(int serial_fd)
{

	
	int fd = serial_fd;
	
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
						compass = vfr.heading;
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

int* g_Prc;
int g_Prc1;
int g_Prc2;
int g_Prc3;
int g_Prc4;
int g_Prc5;

//-------------------Функция проверки существования файла---------------------------------------------
bool exists(const char *fname)
{
	FILE *file;
	if (file = fopen(fname, "r"))
	{
		fclose(file);
		return true;
	}
	return false;
}
//----------------------------------------------------------------------------------------------------

	
int cs2 = spi.InitCS(CS1);
int cs1 = spi.InitCS(CS2);

float ReadSonar(int line)
{
	int val;
	spi.ClrCS(cs1);
	spi.WriteByte(line);
	val = spi.ReadByte();
	spi.SetCS(cs1);

	float k;
	if (line == SONAR1) val *= 5.8;
	else val = val * 1.55 + 2.4;
	
	return val;
}

void Init(void)
{
	g_Prc1 = 50;
	g_Prc2 = 50;
	g_Prc3 = 0;
	g_Prc4 = 50;
	g_Prc5 = 50;

	spi.Init(MISO, MOSI, CLK);
}

void KeyUP(int* prc)
{
	*prc += 10;
	if(*prc > 100)
		*prc = 100;
		
}

void KeyDOWN(int* prc)
{
	*prc -= 10;
	if(*prc < 0)
		*prc = 0;
}

time_t now;

int main(int argc, char const *argv[])
{
	if (argc == 1)
	{
		printf("No params\n");
		return 1;
	}

	Init();

	// debug = true;
	// verbose = true;

	//========================================
	//				MAVLINK
	//========================================
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
	int head;
	while(compass == 0)	serial_wait(fd);
	head = compass;
	printf("Head: %d\n", head);
	// while(1)
	// {
	// 	serial_wait(fd);
	// 	printf("Compass: %d\n", compass);
	// }
	//========================================

	//========================================
	//			ИНИЦИАЛИЗАЦИЯ КАМЕР
	//========================================
	//-------------------------Захват видео с камер (или из файла)----------------------------------------
	#if (defined (CAMERA1))
		CvCapture* capture1 = 0;
		capture1 = cvCreateCameraCapture(NCam1);
		if (!capture1)
		{
	#ifdef OUTPUT_CONSOLE
			printf("Video from the first camera is not captured.");
	#endif
			return EXIT_FAILURE;//Захвата видеопотока не произошло.
		}
	#endif

	#if (defined (CAMERA2))
		CvCapture* capture2 = 0;
		capture2 = cvCaptureFromCAM(NCam2);
		if (!capture2)
		{
	#ifdef OUTPUT_CONSOLE
			printf("Video from the second camera is not captured.");
	#endif
			return EXIT_FAILURE;//Захвата видеопотока не произошло.
		}
	#endif

	#if (defined (FROM_FILE1) || defined (FROM_FILE2))
		//char* file1 = "d:\\OpenCV\\CROCVIDEO\\17.avi";
		char* file1 ="d:\\OpenCV\\CrocFind\\Main1\\3107_2.avi";
	#ifdef OUTPUT_CONSOLE
		printf("File1: %s\n", file1);
	#endif
	#if defined (FROM_FILE1)
		CvCapture* capture1 = 0;
		capture1 = cvCreateFileCapture (file1);
		if (!capture1)
		{
	#ifdef OUTPUT_CONSOLE
			printf("Video from the first camera is not captured.");
	#endif
			return EXIT_FAILURE;//Захвата видеопотока не произошло.
		}
	#endif
	#if defined (FROM_FILE2)
		//char* file2 = "d:\\OpenCV\\CROCVIDEO\\17.avi";
		char* file2 ="d:\\OpenCV\\CrocFind\\Main1\\vid2.avi";
	#ifdef OUTPUT_CONSOLE
		printf("File2: %s\n", file2);
	#endif
		CvCapture* capture2 = 0;
		capture2 = cvCreateFileCapture (file2);
		if (!capture2)
		{
	#ifdef OUTPUT_CONSOLE
			printf("Video from the second camera is not captured.");
	#endif
			return EXIT_FAILURE;//Захвата видеопотока не произошло.
		}
	#endif
	#endif
		//----------------------------------------------------------------------------------------------------

	//--------------Генерация названий видеофайлов и калибровочных изображений для записи-----------------
	#if (defined (WRITE_ALG_CAMERA1) || defined (WRITE_CAMERA1) || defined (WRITE_ALG_CAMERA2) || defined (WRITE_CAMERA2) || defined (WRITE_MARKER) || defined (WRITE_FLOOR))
		const unsigned short int n=30;
		char filename_alg_c1[n];
		char filename_c1[n];
		char filename_alg_c2[n];
		char filename_c2[n];
		char filename_marker[n];
		char filename_floor[n];

		int nVideoFile = 0;
		bool FileName = false;
		while(!FileName)
		{
			sprintf(filename_alg_c1, "/home/Flight_%d_cam1_alg.avi", nVideoFile);
			sprintf(filename_alg_c2, "/home/Flight_%d_cam2_alg.avi", nVideoFile);
			sprintf(filename_c1, "/home/Flight_%d_cam1.avi", nVideoFile);
			sprintf(filename_c2, "/home/Flight_%d_cam2.avi", nVideoFile);
			sprintf(filename_marker, "/home/Flight_%d_cam2_marker.jpg", nVideoFile);
			sprintf(filename_floor, "/home/Flight_%d_cam2_floor.jpg", nVideoFile);

			if (exists(filename_alg_c1) || exists(filename_alg_c2) || exists(filename_c1) || exists(filename_c2) || exists(filename_marker) || exists(filename_floor))
			{
				nVideoFile++;
			}
			else 
			{
				FileName = true;
			}
		}
	#endif
		//----------------------------------------------------------------------------------------------------

		//-------------------------Инициализация структур редакторов для записи видео-------------------------
	#if (defined (WRITE_ALG_CAMERA1) || defined (WRITE_CAMERA1))
		//double fps1 = cvGetCaptureProperty (capture1, CV_CAP_PROP_FPS);
		//Почему-то запись работает только с частотой записи 15 кадров в секунду.
		double fps1 = 15;
		CvSize size1 = cvSize( (int)cvGetCaptureProperty( capture1, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty( capture1, CV_CAP_PROP_FRAME_HEIGHT));
		//cvSetCaptureProperty(capture1,CV_CAP_PROP_BRIGHTNESS,50);
		//cvSetCaptureProperty(capture1,CV_CAP_PROP_CONTRAST,50);
		//cvSetCaptureProperty(capture1,CV_CAP_PROP_SATURATION,50);

	#ifdef WRITE_ALG_CAMERA1
		CvVideoWriter * writer_alg1;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer_alg1 = cvCreateVideoWriter(filename_alg_c1, CV_FOURCC('X','V','I','D'), fps1, size1, 1);
		}
		assert(writer_alg1!=0);
	#endif
	#if defined WRITE_CAMERA1
		CvVideoWriter * writer1;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer1 = cvCreateVideoWriter(filename_c1, CV_FOURCC('X','V','I','D'), fps1, size1, 1);
		}
		assert(writer1!=0);
	#endif

	#endif


	#if (defined (WRITE_ALG_CAMERA2) || defined (WRITE_CAMERA2))
		//double fps2 = cvGetCaptureProperty (capture2, CV_CAP_PROP_FPS);
		double fps2 =15;
		CvSize size2 = cvSize( (int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_HEIGHT));
		// cvSetCaptureProperty(capture2,CV_CAP_PROP_BRIGHTNESS,10);
		// cvSetCaptureProperty(capture2,CV_CAP_PROP_CONTRAST,10);
		// cvSetCaptureProperty(capture2,CV_CAP_PROP_SATURATION,10);
	#if defined WRITE_ALG_CAMERA2
		CvVideoWriter * writer_alg2;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer_alg2 = cvCreateVideoWriter(filename_alg_c2, CV_FOURCC('X','V','I','D'), fps2, size2, 1);
		}
		assert(writer_alg2!=0);
	#endif
	#if defined WRITE_CAMERA2
		CvVideoWriter * writer2;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer2 = cvCreateVideoWriter(filename_c2, CV_FOURCC('X','V','I','D'), fps2, size2, 1);
		}
		assert(writer2!=0);
	#endif

	#endif
		//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------
		//---То, что должно быть объявлено и выполнено до цикла поиска примерно в такой последовательности----
		//----------------------------------------------------------------------------------------------------

		bool WBm=false, Fl=false;//Были ли сперва получены и проанализированы кадры маркера и пола для калибровки.

	#if (defined (CAMERA1) || defined (FROM_FILE1)) 
		IplImage* frame1 = 0;
	#endif
	#if (defined (CAMERA2) || defined (FROM_FILE2)) 
		IplImage* frame2 = 0;
	#endif
	//========================================

	//==========================================
	//				    ARMED
	//==========================================
	if ((argc > 1) && strcmp(argv[1], "test") != 0)
	{	
		printf("Armed\n");
		Armed();
	}

	//==========================================
	// 					ВЗЛЕТ
	//==========================================

	// Для новой батареии - 68, старая - 74
	int thr = 68;
	// if ((argc > 1) && (strcmp(argv[1],"kt3") == 0 || strcmp(argv[1], "sonar") == 0 || strcmp(argv[1], "cam") == 0 || strcmp(argv[1], "up") == 0))
	if ((argc > 1) && strcmp(argv[1],"test") && strcmp(argv[1],"arm"))
	{
		printf("Взлет\n");
		

		THROTTLE_SET(46);
		Delay(2);
		THROTTLE_SET(thr);
		Delay(2);


		////////////////////Получение изображения маркера. Должно быть вызвано однажды над кругом после старта.
		if ((argc > 1) && (strcmp(argv[1], "cam") == 0))
		{
			#if (defined (CAMERA2) || defined (FROM_FILE2))
				frame2 = cvQueryFrame(capture2);
				//frame2 = cvLoadImage("d:\\OpenCV\\CrocFind\\Main1\\Marker2.bmp"); //Взять из файла.
				if(!frame2)
				{
			#ifdef OUTPUT_CONSOLE
					printf("Frame marker for camera 2 is not received.");
			#endif
					return EXIT_FAILURE;
				}
				cvSaveImage(filename_marker, frame2); //Сохранение в обязательном порядке, так как потом считывание следующей функцией.
				printf("Первый кадр\n");		
				WBm=true;
			#endif
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Throttle, Time, Alt
		// Для новой батареии Alt - 54, старая - 58

		LiftUp(thr, 2, 54);
		
	}

	//==========================================
	// 				ТЕСТ СОНАРОВ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "test"))
	{
		float data;
		int asd = 0;
		while(asd < 10)
		{
			asd++;
			data = ReadSonar(SONAR1);
			printf("Нижний сонар: - %f : ", data);
			data = ReadSonar(R_SONAR);
			printf("Правый сонар: - %f : ", data);
			data = ReadSonar(F_SONAR);
			printf("Передний сонар: - %f\n", data);
			for (int i = 0; i < 300; ++i)
			{
				usleep(1000);
			}
		}
	}

	//==========================================
	// 			Взлет и поворот на месте
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "turn"))
	{
		printf("Взлет\n");

		// Throttle, Time, Alt
		LiftUp(70, 2, 63);
		YAW_SET(FR_YAW+30);
		printf("Поворот\n");
		YAW_SET(FR_YAW+35);
		Delay(2);		
		YAW_SET(FR_YAW);
		Delay(1);
	}	

	//==========================================
	// 			ПОЛЕТ ПО КАМЕРЕ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "cam"))
	{
		printf("Полет вперед\n");
		PITCH_SET(FR_PITCH - 15);

		int del = 0;
		while (del < 200)
		{
			del++;
			frame2 = cvQueryFrame(capture2);
			// serial_wait(fd);
			// printf("Compass delay: %d\n", compass);
		}
		PITCH_SET(FR_PITCH - 10);
		////////////////////Функция получения цветов пола. Должна быть вызвана единожды в месте, где точно нет маркера.	
		#if (defined (CAMERA2) || defined (FROM_FILE2))
			frame2 = cvQueryFrame(capture2); //Взять из камеры.
			//frame2 = cvLoadImage("d:\\OpenCV\\CrocFind\\Main1\\Floor111.bmp");	//Взять из файла.
			if(!frame2)
			{
		#ifdef OUTPUT_CONSOLE
				printf("Frame floor for camera 2 is not received.");
		#endif
				return EXIT_FAILURE;
			}
			getFloor (frame2, filename_floor, filename_marker); 
			printf("Второй кадр\n");
			Fl=true;
		#endif
		////////////////////////////////////////////////////////////////////////////////////////////////////

		printf("Полет по камере и сонарам\n");
		float data;
		int dy = 20;
		bool isDownCam;
		bool isOnSonar = true;
		bool isFirstFrwrd = true;
		
		isDownCam = true;
		bool FirstDot = true;
		int dx = 20;
		
		int th = 4;
		int gaz = FR_THROTTLE;

		//------------Цикл обработки кадров и поиска маркера--------------------------------------------------
		while (1)
		{

			// data = ReadSonar(SONAR1);
			// printf("Высота: - %f \t", data);
			// if (data > 100 && data < 150)
			// {	
			// }
			// else
			// if (data < 100)
			// {
			// 	gaz += th;
			// 	if (gaz > 100) gaz = 100;
			// 	printf("Вверх: %d\n", gaz);
			// 	THROTTLE_SET(gaz);
			// }
			// else
			// if (data > 150)
			// {
			// 	gaz -= th;
			// 	if (gaz < 0) gaz = 0;
			// 	printf("Вниз: %d\n", gaz);
			// 	THROTTLE_SET(gaz);
			// }
			// else
			// {
				int koef = 2;
				for (int i = 0; i < 10; ++i)
				{
					serial_wait(fd);
				}
				printf("Compass: %d \t Head: %d", compass, head);
				if ((compass - head) < 0)
				{
					printf("\tВправо YAW\n");
					YAW_SET(FR_YAW - (compass - head) * koef);
					for (int i = 0; i < 50; ++i)
					{
						usleep(1000);
					}
					YAW_SET(FR_YAW);
				}
				else
				if ((compass - head) > 0)	
				{
					printf("\tВлево YAW\n");
					YAW_SET(FR_YAW - (compass - head) * koef);
					for (int i = 0; i < 50; ++i)
					{
						usleep(1000);
					}
					YAW_SET(FR_YAW);
				}

				if (!isDownCam)
				{
					
							//--------------------Первая (верхняя) камера - поиск и при необходимости запись------------------
					#if (defined (CAMERA1) || defined (FROM_FILE1))
							//Получение кадра.
							frame1 = cvQueryFrame(capture1);
							if(!frame1)
							{
					#ifdef OUTPUT_CONSOLE
								printf("Frame for camera 1 is not received.");
					#endif
								return EXIT_FAILURE;
							}
							//Запись необработанного кадра.
					#if (defined(WRITE_CAMERA1) && (defined (CAMERA1) || defined (FROM_FILE1))) 
							cvWriteFrame(writer1, frame1);
					#endif
							//Выполнение поиска.
							//1. Нужно не забыть выполнить сперва калибровку цветов маркера.
							//2. Нужно выполнить затем калибровку цветов пола.
							//3. Выполнять поиск, используя полученное.
							//if (WBm && Fl)//Проверка наличия кадров с маркером и полом.
							//{
							double* xy1 = find1 (frame1);
							double XY10=xy1[0];//Наличие центра маркера в кадре.
							double XY11=xy1[1];//Отклонение от центра.
							double XY12=xy1[2];
							//Вывод результатов поиска.
					#ifdef OUTPUT_CONSOLE_FRAME_RES
							printf ("PPExist = %.4lf, Ux = %.4lf, S = %.4lf \n\n", xy1[0], xy1[1], xy1[2]);
					#endif
							//}
					#ifdef SHOW_WINDOWS
							const char *windowName1 = "WindowCamera1";
							cvNamedWindow(windowName1, CV_WINDOW_AUTOSIZE);
							cvShowImage(windowName1, frame1);
					#endif
					
					
					#endif
							//----------------------------------------------------------------------------------------------------
					
							//----------------------------------------------------------------------------------------------------
							//Реакция на результаты поиска.
					#if (defined (CAMERA1) || defined (FROM_FILE1))
						if (XY10==1)// 
						{
							if (XY11<-0.2)//YAW Влево.
							{
								// char textbuf[10];
								// sprintf(textbuf, "On LEFT");
								// CvFont myFont;
								// cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
								// cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));

								ROLL_SET(FR_ROLL - dx);
								//SetSPI2PWM(chan_buf, 5);
								//Delay(1);

								ROLL_SET(FR_ROLL + dx);
								//SetSPI2PWM(chan_buf, 5);
								// for (int i = 0; i < 500; ++i)
								// {
								// 	usleep(1000);
								// }
								// ROLL_SET(FR_ROLL);
								//SetSPI2PWM(chan_buf, 5);

							}
							else if (XY11>0.2)//YAW Вправо.
							{
								// char textbuf[10];
								// sprintf(textbuf, "On RIGHT");
								// CvFont myFont;
								// cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
								// cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));

								ROLL_SET(FR_ROLL + dx);
								//SetSPI2PWM(chan_buf, 5);
								//Delay(1);

								ROLL_SET(FR_ROLL - dx);
								//SetSPI2PWM(chan_buf, 5);
								// for (int i = 0; i < 500; ++i)
								// {
								// 	usleep(1000);
								// }

								ROLL_SET(FR_ROLL);
								//SetSPI2PWM(chan_buf, 5);
							}
						}
						#ifdef WRITE_ALG_CAMERA1
								printf("Первая камера\n");
								cvWriteFrame(writer_alg1, frame1);
						#endif
					#endif
							//----------------------------------------------------------------------------------------------------
				}

				if (isOnSonar)
				{
					data = ReadSonar(R_SONAR);
					printf("Data R: - %f \t", data);
					if (data > 150 && data < 250)
					{	
						// ROLL_SET(FR_ROLL);
						// for (int i = 0; i < 10; ++i)
						// {
						// 	usleep(1000);
						// }
					}
					else
					if (data < 150)
					{
						printf("Влево\n");
						ROLL_SET(FR_ROLL - dy);
						for (int i = 0; i < 100; ++i)
						{
							usleep(1000);
						}
						//Выравнивание
						ROLL_SET(FR_ROLL);
					}
					else
					if (data > 250)
					{
						printf("Вправо\n");
						ROLL_SET(FR_ROLL + dy);
						for (int i = 0; i < 100; ++i)
						{
							usleep(1000);
						}
						//Выравнивание
						ROLL_SET(FR_ROLL);
					}
				}
						//--------------------Вторая (нижняя) камера - поиск и при необходимости запись-----------------------
				#if (defined (CAMERA2) || defined (FROM_FILE2))
						frame2 = cvQueryFrame(capture2);
						if(!frame2)
						{
				#ifdef OUTPUT_CONSOLE
							printf("Frame for camera 2 is not received.");
				#endif
							return EXIT_FAILURE;
						}
						//Запись необработанного кадра.
				#if (defined(WRITE_CAMERA2)) 
						cvWriteFrame(writer2, frame2);
				#endif
						//Выполнение поиска.
						//1. Нужно не забыть выполнить сперва калибровку цветов маркера.
						//2. Нужно выполнить затем калибровку цветов пола.
						//3. Выполнять поиск, используя полученное.
						//if (WBm && Fl)//Проверка наличия кадров с маркером и полом.
						//{
						unsigned short int* xy2 = find2 (frame2);
						unsigned short int XY20=xy2[0];
						unsigned short int XY21=xy2[1];
						unsigned short int XY22=xy2[2];
						unsigned short int XY23=xy2[3];
						unsigned short int XY24=xy2[4];
						unsigned short int XY25=xy2[5];
						//Вывод результатов поиска.
				#ifdef OUTPUT_CONSOLE_FRAME_RES
						printf ("PP = %d, Up = %d, Right = %d, Down = %d, Left = %d \n\n", xy2[0], xy2[1], xy2[2],  xy2[3], xy2[4]);
				#endif
						//}
				#ifdef SHOW_WINDOWS
						const char *windowName2 = "WindowCamera2";
						cvNamedWindow(windowName2, CV_WINDOW_AUTOSIZE);
						cvShowImage(windowName2, frame2);
				#endif


				#endif
						//----------------------------------------------------------------------------------------------------

				//----------------------------------------------------------------------------------------------------
				//Реакция на результаты поиска.
				#if (defined (CAMERA2) || defined (FROM_FILE2))

					if (XY20 > 0)
					{
						isDownCam = true;
						isOnSonar = false;
						if (FirstDot)
						{
							printf("Сонар отключен\n");
							FirstDot = false;
						}
						if (XY20 > 2)
						{
							if (isFirstFrwrd)
							{
								isFirstFrwrd = false;
								PITCH_SET(FR_PITCH + 8);
								Delay(1);
	
								PITCH_SET(FR_PITCH);
							}
						}
						if (XY20 == 4 || XY25 == 1/*|| (XY21 == 1 && XY23 == 1)  || (XY22 == 1 && XY24 == 1)*/)
						{
							char textbuf[10];
							sprintf(textbuf, "LANDING");
							CvFont myFont;
							cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
							if (XY20 == 4) //Посадка по 4 точкам и перпендикулярным отрезкам. Желтый текст.
							{
								cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));
							}
							else if (XY25==1)//Посадка только по перпендикулярным отрезкам. Красный текст.
							{
								cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,0));
							}
							cvWriteFrame(writer_alg2, frame2);
							break;
						}
						else
						{
							isOnSonar = true;
							FirstDot = true;
						}

						// Движение вправо
						if (XY21 == 1 && XY23 == 0)
						{
							ROLL_SET(FR_ROLL + dx);
							for (int i = 0; i < 50; ++i)
							{
								usleep(1000);
							}
							ROLL_SET(FR_ROLL);
						}
						
						// Движение влево
						if (XY23 == 1 && XY21 == 0)
						{
							ROLL_SET(FR_ROLL - dx);
							for (int i = 0; i < 50; ++i)
							{
								usleep(1000);
							}
							ROLL_SET(FR_ROLL);
						}
						
						// Движение вперед
						if (XY24 == 1 && XY22 == 0)
						{
							// char textbuf3[12];
							// sprintf(textbuf3, "Frwrd ROLL");
							// CvFont myFont3;
							// cvInitFont(&myFont3,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
							// cvPutText(frame2,textbuf3,cvPoint(frame2->width/4,frame2->height/2+20),&myFont3,cvScalar(0,255,255));

							PITCH_SET(FR_PITCH - dx);
							// for (int i = 0; i < 200; ++i)
							// {
							// 	usleep(1000);
							// }
							PITCH_SET(FR_PITCH);

						}
						
						// Движение назад
						if (XY22 == 1 && XY24 == 0)
						{
							// char textbuf4[12];
							// sprintf(textbuf4, "Back ROLL");
							// CvFont myFont4;
							// cvInitFont(&myFont4,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
							// cvPutText(frame2,textbuf4,cvPoint(frame2->width/4,frame2->height/2+20),&myFont4,cvScalar(0,255,255));

							PITCH_SET(FR_PITCH + dx);
							for (int i = 0; i < 50; ++i)
							{
								usleep(1000);
							}
							PITCH_SET(FR_PITCH);
						}

					}
				// if (XY20==4) //Посадка.
				// {
				// 	char textbuf[10];
				// 	sprintf(textbuf, "LANDING");
				// 	CvFont myFont;
				// 	cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
				// 	cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));
				// }
				// /*if () //Roll влево. 
				// {

				// }*/

				#ifdef WRITE_ALG_CAMERA2
						printf("Вторая камера\n");
						system("date");
						cvWriteFrame(writer_alg2, frame2);
				#endif
				#endif
						char c = cvWaitKey(33);
						//----------------------------------------------------------------------------------------------------
			// } //else
		}

	}

	//==========================================
	// 			ПОЛЕТ ПО СОНАРАМ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "sonar"))
	{	
		printf("Полет по сонарам\n");
		float data;
		int dy = 5;
		while(1)
		{
			data = ReadSonar(R_SONAR);
			printf("Data R: - %f \t", data);
			if (data > 150 && data < 250)
			{	
				// ROLL_SET(FR_ROLL);
				for (int i = 0; i < 10; ++i)
				{
					usleep(1000);
				}
			}
			else
			if (data < 150)
			{
				printf("Влево\n");
				ROLL_SET(FR_ROLL - dy);
				for (int i = 0; i < 500; ++i)
				{
					usleep(1000);
				}
				//Торможение
				/*ROLL_SET(FR_ROLL + dy);
				for (int i = 0; i < 250; ++i)
				{
					usleep(1000);
				}*/
				//Выравнивание
				ROLL_SET(FR_ROLL);
			}
			else
			if (data > 250)
			{
				printf("Вправо\n");
				ROLL_SET(FR_ROLL + dy);
				for (int i = 0; i < 500; ++i)
				{
					usleep(1000);
				}
				//Торможение
				/*ROLL_SET(FR_ROLL - dy);
				for (int i = 0; i < 250; ++i)
				{
					usleep(1000);
				}*/
				//Выравнивание
				ROLL_SET(FR_ROLL);
			}
			data = ReadSonar(F_SONAR);
			printf("Data F: - %f \t", data);
			if (data <= 250)
			{
				PITCH_SET(FR_PITCH + 4);
				Delay(1);
				PITCH_SET(FR_PITCH);
				break;
			}
		}

		//Поворот по компасу на 90 градусов
		// while(compass == 0)	serial_wait(fd);

		/*printf("Compass1: %d\n", compass);
		int deg;
		if (compass < 90) deg = 360 - (90 - compass);
		else deg = compass - 90;
		printf("Азимут первого поворота: %d\n", deg);
		int dc = 5;
		YAW_SET(FR_YAW-4);
		while (compass > (deg + dc))
		{
			serial_wait(fd);
		}		
		YAW_SET(FR_YAW);
		PITCH_SET(FR_PITCH - 6);
		Delay(2);
		PITCH_SET(FR_PITCH + 2);
		Delay(1);
		PITCH_SET(FR_PITCH);*/
		//Конец поворта по компасу на 90 градусов
		// serial_wait(fd);
	}

	//==========================================
	// 				ПОЛЕТ ВПЕРЕД
	//==========================================
		int asd = 0;
		time_t dif;
	if ((argc > 1) && !strcmp(argv[1], "kt3"))
	{
		printf("Полет Г\n");

		time_t diff = time(0);
		now = time(0);

		int diffUR = 0;
		int diffUF = 0;

		PITCH_SET(FR_PITCH - 10);
		while ((now - diff) < 20)
		{
			now = time(0);

				int koef = 4;
				// for (int i = 0; i < 10; ++i)
				// {
					serial_wait(fd);
				// }
				printf("Compass: %d Head: %d\t", compass, head);
				if ((compass - head) < 0)
				{
					int kr = 1;
					printf("Вправо YAW\t");
					if ((int)(FR_YAW - (compass - head) * koef * kr) < 0)
					{
						YAW_SET(0);	
					}
					else
					if ((int)(FR_YAW - (compass - head) * koef * kr) > 100)
					{
						YAW_SET(100);	
					}
					else YAW_SET(FR_YAW - (compass - head) * koef * kr);
					// for (int i = 0; i < 50; ++i)
					// {
						usleep(1000);
					// }
					YAW_SET(FR_YAW);
				}
				else
				if ((compass - head) > 0)	
				{
					int kl = 1;
					printf("Влево YAW\t");
					if ((int)(FR_YAW - (compass - head) * koef * kl) < 0)
					{
						YAW_SET(0);	
					}
					else
					if ((int)(FR_YAW - (compass - head) * koef * kl) > 100)
					{
						YAW_SET(100);	
					}
					else YAW_SET(FR_YAW - (compass - head) * koef * kl);
					// for (int i = 0; i < 50; ++i)
					// {
						usleep(1000);
					// }
					YAW_SET(FR_YAW);
				}
				else
				{
					printf("Середина YAW\t\n");
				}

			float data3;
			data3 = ReadSonar(R_SONAR);
			printf("Data R: - %f ", data3);
			printf("Разница R: %f\t", data3 - diffUR);
			diffUR = data3;
			if (data3 > 150 && data3 < 250)
			{	
				usleep(1000);
			}
			else
			if (data3 < 150)
			{
				printf("Влево ");
				// ROLL_SET(FR_ROLL - 10);
				usleep(1000);
				//Выравнивание
				// ROLL_SET(FR_ROLL);
			}
			else
			if (data3 > 250)
			{
				printf("Вправо\t");
				// ROLL_SET(FR_ROLL + 10);
				usleep(1000);
				//Выравнивание
				// ROLL_SET(FR_ROLL);
			}

			data3 = ReadSonar(F_SONAR);
			printf("Data F: - %f ", data3);
			printf("Разница F: %f", data3 - diffUF);
			diffUF = data3;
			if (data3 < 380)
			{
				printf("Посадка\n");
				PITCH_SET(FR_PITCH + 10);
				break;
			}
			else
			{
				usleep(1000);
			}

			printf("\n");

		}

		Delay(1);
		PITCH_SET(FR_PITCH);	
		// Disarmed();
		// return 0;

		// dif = time(0);
		// PITCH_SET(FR_PITCH - 11);
		// Delay(5);
		// PITCH_SET(FR_PITCH + 9);
		// Delay(1);
		// PITCH_SET(FR_PITCH);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);

		// dif = time(0);
		// printf("Hover\n");
		// Delay(2);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);



		// dif = time(0);
		// ROLL_SET(FR_ROLL - 6);
		// Delay(3);
		// ROLL_SET(FR_ROLL + 13);
		// Delay(1);
		// for (int i = 0; i < 500; ++i)
		// {
		// 	usleep(1000);
		// }
		// ROLL_SET(FR_ROLL);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);

		// dif = time(0);
		// printf("Hover\n");
		// Delay(1);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);




		// dif = time(0);
		// ROLL_SET(FR_ROLL + 12);
		// Delay(5);
		// ROLL_SET(FR_ROLL - 6);
		// Delay(1);
		// for (int i = 0; i < 500; ++i)
		// {
		// 	usleep(1000);
		// }
		// ROLL_SET(FR_ROLL);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);

		// dif = time(0);
		// printf("Hover\n");
		// Delay(1);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);

		// dif = time(0);
		// PITCH_SET(FR_PITCH + 4);
		// Delay(5);
		// PITCH_SET(FR_PITCH - 7);
		// Delay(1);
		// PITCH_SET(FR_PITCH);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);

		// dif = time(0);
		// printf("Hover\n");
		// Delay(2);
		// now = time(0);
		// printf("Время: %d с\n", now - dif);
	}

	//==========================================
	// 					ПОСАДКА
	//==========================================
	if ((argc > 1) && (strcmp(argv[1], "test")) && (strcmp(argv[1], "arm")))
	{
		printf("Посадка\n");
		system("date");

		// PITCH_SET(FR_PITCH + 4);
		// Delay(1);
		// PITCH_SET(FR_PITCH);

		// Throttle, Time
		LiftDown(thr, 1);
		printf("Двигатели отключены\n");
		system("date");		
		asd = 0;
		while (asd < 3)
		{
			asd++;
			serial_wait(fd);
			printf("Compass: %d\n", compass);	
		}

	}
	close_port(fd);
	//==========================================
	//		   		РУЧНОЙ РЕЖИМ
	//==========================================
	if (argc > 1 && strcmp(argv[1], "arm") == 0)
	{
		printf("Ручной режим\n");
		initscr();	
		raw();
		keypad(stdscr, TRUE);
		noecho();
		
		printw("GO!");
	
		bool loop;
		loop = true;
		char ch;
		int chan;
		chan = 1;
		g_Prc = &g_Prc1;
		int g_chan[4];
		// ROLL
		g_chan[0] = 50;
		// PITCH
		g_chan[1] = 50;
		// THROTTLE
		g_chan[2] = 0;
		// YAW
		g_chan[3] = 50;
		while( loop == true)
		{
			ch = getch();
			switch (ch)
			{
				// case 'w': KeyUP(g_Prc); printw("%d ", *g_Prc); break;
				// case 's': KeyDOWN(g_Prc); printw("%d ", *g_Prc); break;
				case 'w': 
				{
					g_chan[2] += 10;
					if (g_chan[2] >= 100) g_chan[2] = 100;
					printw("T:%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 's':
				{
					g_chan[2] -= 10;
					if (g_chan[2] <= 0) g_chan[2] = 0;
					printw("T:%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'a': 
				{
					g_chan[0] -= 1;
					if (g_chan[0] <= 0) g_chan[0] = 0;
					printw("Y:%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					//SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'd':
				{
					g_chan[0] += 1;
					if (g_chan[0] >= 100) g_chan[0] = 100;
					printw("Y:%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				
				case 'k': 
				{
					g_chan[1] += 1;
					if (g_chan[1] >= 100) g_chan[1] = 100;
					printw("P:%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'i':
				{
					g_chan[1] -= 1;
					if (g_chan[1] <= 0) g_chan[1] = 0;
					printw("P:%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'j': 
				{
					g_chan[3] -= 1;
					if (g_chan[3] <= 0) g_chan[3] = 0;
					printw("R:%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					//SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'l':
				{
					g_chan[3] += 1;
					if (g_chan[3] >= 100) g_chan[3] = 100;
					printw("R:%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'f': printw("Mode stabilize. "); MODE_SET(0);   break;
				case 'g': printw("Mode Althold. "); MODE_SET(100); break;
				case 'z': printw("ThrUP. "); THROTTLE_SET(100); /*SetSPI2PWM(chan_buf, 5);*/ break;
				case 'c': printw("ThrDOWN. "); THROTTLE_SET(0); /*SetSPI2PWM(chan_buf, 5);*/ break;
				case '1': printw("Roll: "); chan = 1; break;
				case '2': printw("Pitch: "); chan = 2; break;
				case '3': printw("Throttle: "); chan = 3; break;
				case '4': printw("Yaw: "); chan = 4; break;
				case '5': printw("Mode: "); chan = 5; break;
				case '[': printw("Start... "); THROTTLE_SET(0); YAW_SET(100); /*SetSPI2PWM(chan_buf, 5);*/ Delay(4); YAW_SET(50); /*SetSPI2PWM(chan_buf, 5);*/ printw("Ok. "); break;
				case ']': printw("Stop... "); THROTTLE_SET(0); YAW_SET(0); /*SetSPI2PWM(chan_buf, 5);*/ Delay(4); YAW_SET(50); /*SetSPI2PWM(chan_buf, 5);*/ printw("Ok. "); break;
				case 'r': {
						printw("Reset... ");
						ROLL_SET(50);
						PITCH_SET(50);
						THROTTLE_SET(0);
						YAW_SET(50);
						MODE_SET(0);
						//SetSPI2PWM(chan_buf, 5);
						printw("Ok. ");
						break;
				}
				case 'q':
				case 'Q':loop = false;
			}
			
			switch (chan)
			{
				//case 1: g_Prc = &g_Prc1; ROLL_SET(*g_Prc); break;
				//case 2: g_Prc = &g_Prc2; PITCH_SET(*g_Prc); break;
				//case 3: g_Prc = &g_Prc3; THROTTLE_SET(*g_Prc); break;
				//case 4: g_Prc = &g_Prc4; YAW_SET(*g_Prc); break;
				//case 5: g_Prc = &g_Prc5; MODE_SET(*g_Prc); break;
			}
			//SetSPI2PWM(chan_buf, 5);
		}
		refresh();
		getch();
		endwin();
	}

	//==========================================
	// 				  DISARMED
	//==========================================
	printf("Disarmed\n");
	Disarmed();

	//-------------Освобождение памяти, разрушение окон и т.д.--------------------------------------------
	if ((argc > 1) && (strcmp(argv[1], "cam") == 0))
	{
		#if (defined (CAMERA1) || defined (FROM_FILE1))
			cvReleaseCapture( &capture1 );
		#ifdef WRITE_ALG_CAMERA1
			cvReleaseVideoWriter(&writer_alg1);
		#endif
		#ifdef WRITE_CAMERA1
			cvReleaseVideoWriter(&writer1);
		#endif
		#endif
		
		#if (defined (CAMERA2) || defined (FROM_FILE2))
		#ifdef WRITE_ALG_CAMERA2
			cvReleaseVideoWriter(&writer_alg2);
		#endif
		#ifdef WRITE_CAMERA2
			cvReleaseVideoWriter(&writer2);
		#endif
		#endif
		
		#ifdef SHOW_WINDOWS
			cvDestroyAllWindows();
		#endif
		//----------------------------------------------------------------------------------------------------
	}
	return 0;
}

void Delay(int time)
{
	for (int i = 0; i < time*1000; i++) usleep(1000);
}