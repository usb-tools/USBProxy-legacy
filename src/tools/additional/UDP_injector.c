/*
 * Copyright 2014 David Formby
 *
 * This file is part of USBProxy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * Sample UDP client
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/***** after studying the Injector_UDP.cpp code, it looks like it expects it in the following format*/
/*buf[0]=endpoint*/
/*buf[1]b0 = flag for filter bool (0=True, 1=False)*/
/*buf[1]b1 = flag for transmit bool (0=True, 1=False)*/
/*buf[2] = MSB for usblen*/
/*buf[3] = LSB for usblen*/
/*starting at buf[4] is actual data*/

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   unsigned char* chunk;
   FILE* file;
   int index =0;
   unsigned char c;
   char* file_bytes;
   int size, chunk_size;
   int i,j;
   int bytes;
   int packets_sent = 0;
   
   char endpoint_str[6] = {0};
   unsigned char endpoint = 0;
   char usblen_str[12] = {0};
   unsigned short usblen;
   char data_bytes[3];
   
   file = fopen("test_file.txt","r");
   fseek(file, 0L, SEEK_END);
   size = ftell(file);
   fseek(file, 0L, SEEK_SET);
   file_bytes = malloc(size*sizeof(char));
   i = 0;
   while ((c = fgetc(file)) != EOF && i < size) {
       file_bytes[i] = c;
       //printf("%x",c);
       i++;
   }

   if (argc != 2)
   {
      printf("usage:  udpcli <IP address>\n");
      fclose(file);
      free(file_bytes);
      exit(1);
   }

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(12345);

   while(index < size) {
        //parse the next usb packet in log file
        if (isalnum(file_bytes[index])) {
            j = 0;
            while (file_bytes[index+j] != '[' && j < 5) {
                endpoint_str[j] = file_bytes[index+j];
                j++;
            }
            endpoint_str[j] = '\0';
            endpoint = strtol(endpoint_str, NULL, 16);
            printf("\nep=%.2X ",endpoint);
            index = index+j;
        } else {
            //something isn't right if it gets here
            printf("something is wrong, found %c in front", file_bytes[index]);
            index++;
	    fclose(file);
	    free(file_bytes);
            return 1;
        }
        if (file_bytes[index] == '[') {
            index++;
            j = 0;
            while (file_bytes[index+j] != ']' && j < 11) {
                usblen_str[j] = file_bytes[index+j];
                j++;
            }
            usblen_str[j] = '\0';
            usblen = atoi(usblen_str);
            index = index+j+2;

        }
        else {
            printf("something is wrong");
	    fclose(file);
	    free(file_bytes);
            return 1;
        }	
        
        chunk = (unsigned char*) malloc(usblen+4);
        chunk[0] = endpoint;
        chunk[1] = 0x00;
        chunk[2] = (usblen & 0xFF00) >> 8;
        chunk[3] = usblen & 0x00FF;
        printf("len= %d\n", usblen);

        if (chunk[0] == 0x02) {
	    bytes = 0;
	    while (bytes < usblen) {
		while (isspace(file_bytes[index])) {
			index++;
		}
                sscanf(&file_bytes[index], "%s", &data_bytes);
                c = (unsigned char) strtol(data_bytes, NULL, 16);
//               printf("%.2X ", c);
                chunk[4+bytes] = c;
                bytes++;
                index = index + 3;
            }
        }
        chunk_size = usblen + 4;
        //move index to next USB packet
        while (index < size && file_bytes[index] != '[') {
            index++;
        }
        if (file_bytes[index] == '[')
            index = index - 2;


        if (chunk[0] == 0x02) {
            //maybe need to log timestamps and simulate timing
            //by using sleep?
//        sleep(2);
        for (i = 0; i < chunk_size; i++) {
		printf("%.2X ", chunk[i]);
	}
        sendto(sockfd,chunk,chunk_size,0,
         (struct sockaddr *)&servaddr,sizeof(servaddr));
         packets_sent++;
         printf("Sent packet to ep=0x02, total=%d packets\n", packets_sent);
        }
        free(chunk);
   }
   fclose(file);
   free(file_bytes);
   return 0;
}
