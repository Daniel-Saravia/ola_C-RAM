/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * ola-latency.cpp
 * Measures the round trip time of olad and provides stats on exit
 * Copyright (C) 2005  Simon Newton
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <getopt.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include <ola/ola.h>


#define CHANNELS 512

int universe = 0;


static ola_con con ;
uint8_t	dmx[CHANNELS] ;

struct timeval tv1;

long worst = 0;
long best = 9999999 ;
long count = 0; 
long total = 0 ;

int term = 0;


/*
 * Terminate cleanly on interrupt
 */
static void sig_interupt(int signo) {
	term = 1 ;
}

/*
 * Set up the interrupt signal
 *
 * @return 0 on success, non 0 on failure
 */
static int install_signal() {
	struct sigaction act, oact;

	act.sa_handler = sig_interupt;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGINT, &act, &oact) < 0) {
		printf("Failed to install signal") ;
		return -1 ;
	}
	
	if (sigaction(SIGTERM, &act, &oact) < 0) {
		printf("Failed to install signal") ;
		return -1 ;
	}

	return 0;
}


/*
 * called on recv
 */
int dmx_handler(ola_con c, int uni, int length, uint8_t *sdmx, void *d ) {
	int len = length > CHANNELS ? CHANNELS : length ;
	struct timeval tv2 ;
	long delay ;

	gettimeofday(&tv2, NULL) ;
	delay = tv2.tv_usec - tv1.tv_usec ;
	printf("rtt %ld usec \n", delay) ;
	
	worst = delay > worst ? delay : worst;
	best = delay < best ? delay : best ;

	total += delay ;
	count ++ ;
	
	if(memcmp(dmx, sdmx,len)) {
		printf("no match!\n") ;
	}

	return 0 ;
}


int main (int argc, char *argv[]) {
	int optc ;
	int ola_sd ;
//	struct timeval tv2 ;
	
	install_signal() ;
	
	memset(dmx, 0x00, CHANNELS) ;

	// parse options 
	while ((optc = getopt (argc, argv, "u:")) != EOF) {
		switch (optc) {
			case 'u':
			universe= atoi(optarg) ;
			break ;
		default:
			break;
		}
	}

	/* set up ola connection */
	con = ola_connect() ; ;
	
	if(con == NULL) {
		printf("Unable to connect\n") ;
		return 1 ;
	}

	if(ola_set_dmx_handler(con, dmx_handler, NULL) ) {
		printf("Failed to install handler\n") ;
		return 1 ;
	}

	if(ola_reg_uni(con, universe, 1) ) {
		printf("REgister uni %d failed\n", universe) ;
		return 1 ;
	}

	// store the sds
	ola_sd = ola_get_sd(con) ;
  
	/* main loop */
	while (! term) {
		int n, max;
		fd_set rd_fds;
		struct timeval tv;

		FD_ZERO(&rd_fds);
		FD_SET(ola_sd, &rd_fds) ;

		max = ola_sd ;

		tv.tv_sec = 0;
		tv.tv_usec = 40000;

		n = select(max+1, &rd_fds, NULL, NULL, &tv);
		if(n>0) {
			if (FD_ISSET(ola_sd, &rd_fds) ) {
//				gettimeofday(&tv2, NULL) ;
//				printf(" got read %ld %ld\n", tv2.tv_sec, tv2.tv_usec) ;
					
	    		ola_sd_action(con,0);
			}
		}
		if(n==0) {
			gettimeofday(&tv1, NULL) ;
//			printf("sending %ld %ld\n", tv1.tv_sec, tv1.tv_usec) ;
			ola_send_dmx(con, universe, dmx, CHANNELS) ;
		}

	}
	ola_disconnect(con) ;

	printf("Best %ld , Worst %ld, Avg %ld\n", best, worst , total/count) ;

	return 0;
}