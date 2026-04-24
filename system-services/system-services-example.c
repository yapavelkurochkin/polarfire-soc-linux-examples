// SPDX-License-Identifier: MIT
/*
 * MSS system services example for the Microchip PolarFire SoC
 *
 * Copyright (c) 2021 Microchip Technology Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define BUFFSIZE 4096

#define SERIAL_COMMAND 0u
#define SERIAL_MSG_SIZE 0u
#define SERIAL_RESP_SIZE 16u
#define SERIAL_OFFSET 0u
#define SERIAL_DEVICE "/dev/mpfs_generic_service"

#define USERCODE_COMMAND 1u
#define USERCODE_RESP_SIZE 4u

#define DESINFO_COMMAND 2u
typedef struct ss_design_info {
	uint8_t id[32];
	uint16_t version;
	uint16_t backlevel;
} __attribute__((packed)) ss_design_info_t;

#define DIGEST_COMMAND 0x4u
#define DIGEST_MSG_SIZE 0u
#define DIGEST_RESP_SIZE 576u
#define DIGEST_OFFSET 0u
#define DIGEST_DEVICE "/dev/mpfs_generic_service"
#define DIGEST_LINE_LEN 33u
#define DIGEST_SERIAL_OFFSET 50u

#define SIGN_COMMAND 26u
#define SIGN_MSG_SIZE 48u
#define SIGN_RESP_SIZE 104u
#define SIGN_OFFSET 0u
#define SIGN_DEVICE "/dev/mpfs_generic_service"

#define SS_DEVICE "/dev/mpfs_generic_service"
typedef struct cmd {
    unsigned int command,
				msg_size, 
				resp_size, 
				send_offset, 
				response_offset;
} ss_cmd_t;

static void display_output(char *data, unsigned int byte_length)
{
    unsigned int inc;
    char byte = 0;
    char buffer[BUFFSIZE];
    char *bufferp = buffer;

	for (inc = 0; inc < byte_length; inc++)
	{
		if(inc%32 == 0 && inc != 0)
		{
			bufferp += sprintf(bufferp, "\r\n");
		}
		byte = data[32*(1+inc/32) - inc%32 - 1];
		bufferp += sprintf(bufferp, "%02x", byte);
    }

    printf("%s\n", buffer);
}


// resp should be of a proper response size
static int ss_request(const ss_cmd_t* cmd, void *resp)
{
    FILE *fptr = NULL;
	if (access(SS_DEVICE, F_OK))
	{
		printf("Error! file doesnt exist\n");
		exit(1);
	}
	else if ((fptr = fopen(SS_DEVICE, "w")) == NULL)
	{
		printf("Error! opening file\n");
		exit(1);
	}

	fprintf(fptr, "%c%c%c%c%c%c%c%c%c",
			cmd->command,
			(char)cmd->msg_size, (char)(cmd->msg_size >> 8),
			(char)cmd->resp_size, (char)(cmd->resp_size >> 8),
			(char)cmd->send_offset, (char)(cmd->send_offset >> 8),
			(char)cmd->response_offset, (char)(cmd->response_offset >> 8));

	fclose(fptr);
	if ((fptr = fopen(SS_DEVICE, "r")) == NULL)
	{
		printf("Error! opening file\n");
		exit(1);
	}

	int ret = 0;	
	uint8_t buf[4096] = {0};
	ret = fread(buf, cmd->resp_size + 1/*status*/, 1, fptr);
	if (ret != 1)
	{
		printf("Error reading status\n");
		exit(1);
	} else if (buf[0] != 0) {
		printf("Request %d failed\n", cmd->command);
		exit(1);
	}

	memcpy(resp, &buf[1], cmd->resp_size);

	fclose(fptr);
	return 0;
}

int main()
{
    unsigned char inbuff[BUFFSIZE];
    unsigned int command, msg_size, resp_size, send_offset, response_offset;
    FILE *fptr;
    char chr;
    int i;

    for (;/*ever*/;)
    {
        printf("PolarFire SoC system services example program.\r\nPress:\r\n");
        printf("1 - to show the FPGA device serial number\r\n"
				"2 - to show the FPGA device digests\r\n"
				"3 - continuously output random numbers from the TRNG, until ctrl+c\r\n"
				"4 - to request an ECDSA signature\r\n"
				"5 - to request design info\r\n"
				"6 - to request usercode (Silicon ID)\r\n"
				"e - to exit this program\r\n"
				);
        chr = getchar();
        getchar();

        switch (chr)
        {
        case '1':
            if (access(SERIAL_DEVICE, F_OK))
            {
                printf("Error! file doesnt exist\n");
                exit(1);
            }
            else if ((fptr = fopen(SERIAL_DEVICE, "w")) == NULL)
            {
                printf("Error! opening file\n");
                exit(1);
            }

            command = SERIAL_COMMAND;
            msg_size = SERIAL_MSG_SIZE;
            resp_size = SERIAL_RESP_SIZE;
            send_offset = SERIAL_OFFSET;
            response_offset = send_offset;
            fprintf(fptr, "%c%c%c%c%c%c%c%c%c",
                    command,
                    (char)msg_size, (char)(msg_size >> 8),
                    (char)resp_size, (char)(resp_size >> 8),
                    (char)send_offset, (char)(send_offset >> 8),
                    (char)response_offset, (char)(response_offset >> 8));
            fclose(fptr);
            if ((fptr = fopen(SERIAL_DEVICE, "r")) == NULL)
            {
                printf("Error! opening file\n");
                exit(1);
            }
            fscanf(fptr, "%17s", inbuff);
            if (*inbuff){
                printf("Error getting serial number: %02x", *inbuff);
            }
            else {
                printf("Icicle Kit Serial Number: \n");
                for(i = 1; i < SERIAL_RESP_SIZE + 1; i++){
                    printf("%02x", inbuff[i]);
                }
                printf("\n");
            }
            fclose(fptr);
            break;
        case '2':
            if (access(DIGEST_DEVICE, F_OK))
            {
                printf("Error! file doesnt exist\n");
                exit(1);
            }
            else if ((fptr = fopen(DIGEST_DEVICE, "w")) == NULL)
            {
                printf("Error! opening file\n");
                exit(1);
            }
            command = DIGEST_COMMAND;
            msg_size = DIGEST_MSG_SIZE;
            resp_size = DIGEST_RESP_SIZE;
            send_offset = DIGEST_OFFSET;
            response_offset = send_offset;
            fprintf(fptr, "%c%c%c%c%c%c%c%c%c",
                    command,
                    (char)msg_size, (char)(msg_size >> 8),
                    (char)resp_size, (char)(resp_size >> 8),
                    (char)send_offset, (char)(send_offset >> 8),
                    (char)response_offset, (char)(response_offset >> 8));
            fclose(fptr);
            if ((fptr = fopen(DIGEST_DEVICE, "r")) == NULL)
            {
                printf("Error! opening file\n");
                exit(1);
            }
            printf("pfsoc fpga digest:\n");
            size_t ret;
            ret = fread(inbuff, 1, 1, fptr);
            if (*inbuff){
                printf("Error getting digest: %02x", *inbuff);
            }
            do {
                ret = fread(inbuff, 1, DIGEST_LINE_LEN, fptr);
                for(i = 0; i < DIGEST_LINE_LEN; i++){
                    printf("%02x", inbuff[i]);
                }
                printf("\n");
            } while (ret == DIGEST_LINE_LEN);
            if (feof(fptr))
            {
                printf("\n");
            }
            fclose(fptr);
            break;
        case '3':
            if ((fptr = fopen("/dev/hwrng", "r")) == NULL)
            {
                printf("Error! opening file\n");
                exit(1);
            }
            for (;/*ever*/;)
            {
                fread(inbuff, 32, 1, fptr);
                display_output(inbuff, 32);
            }
            fclose(fptr);
            break;
        case '4':
            if (access(SIGN_DEVICE, F_OK))
            {
                printf("Error! file doesnt exist\n");
                exit(1);
            }
            else if ((fptr = fopen(SIGN_DEVICE, "w")) == NULL)
            {
                printf("Error! opening file\n");
                exit(1);
            }
            /* write 48 byte hash to the device file */
            command = SIGN_COMMAND;
            msg_size = SIGN_MSG_SIZE;
            resp_size = SIGN_RESP_SIZE;
            send_offset = SIGN_OFFSET;
            response_offset = SIGN_MSG_SIZE + send_offset;
            fprintf(fptr, "%c%c%c%c%c%c%c%c%c47f05d367b0c32e438fb63e6cf4a5f35c2aa2f90dc7543f8",
                    command,
                    (char)msg_size, (char)(msg_size >> 8),
                    (char)resp_size, (char)(resp_size >> 8),
                    (char)send_offset, (char)(send_offset >> 8),
                    (char)response_offset, (char)(response_offset >> 8)
                    );
            fclose(fptr);
            if ((fptr = fopen(SIGN_DEVICE, "r")) == NULL)
            {
                printf("Error! opening signature device file\n");
                exit(1);
            }
            /* read back 104 byte DER format signature */
            ret = fread(inbuff, SIGN_RESP_SIZE + 1, 1, fptr); /* + 1 for status */
            if (*inbuff){
                printf("Error getting signature: %02x", *inbuff);
            }
            else {
                printf("status signature:\n");
                for(i = 1; i < SIGN_RESP_SIZE + 1; i++){
                    printf("%02x", inbuff[i]);
                }
                printf("\n");
            }
            fclose(fptr);
            break;
		case '5':
			{
				ss_cmd_t cmd;
				cmd.command = DESINFO_COMMAND;
				cmd.msg_size = 0;
				cmd.resp_size = sizeof(ss_design_info_t);
				cmd.send_offset = 0;	
				cmd.response_offset = 0;	

				ss_design_info_t info;
				if (0 == ss_request(&cmd, &info))
				{
					printf("version %05d\n", info.version);
				}
				printf("\n");
			}
			break;
		case '6':
			{
				ss_cmd_t cmd;
				cmd.command = USERCODE_COMMAND;
				cmd.msg_size = 0;
				cmd.resp_size = USERCODE_RESP_SIZE;
				cmd.send_offset = 0;	
				cmd.response_offset = 0;	

				uint32_t usercode;
				if (0 == ss_request(&cmd, &usercode))
				{
					printf("usercode %d\n", usercode);
				}
				printf("\n");
			}
			break;
        case 'e':
            return 0;

        default:
            break;
        }
    }
    return 0;
}
