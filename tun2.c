/* tun2.c
 *
 * Sample code for creating two TUN interfaces that are cross-connected
 * at the userspace (socket) level.
 *
 *              Linux Networking
 *
 *              |              ^
 *              v              |
 *         egress (TX)    ingress (RX)
 *              |              ^
 *              v              |
 *            +------------------+
 *            |       TUN1       |
 *            +------------------+
 *              |              ^        \
 *              v              |         |
 *          sock read      sock write    |
 *              |              ^         | userspace
 *              v              |         |
 *          sock write     sock read     |
 *              |              ^         |
 *              v              |        /
 *            +------------------+
 *            |       TUN2       |
 *            +------------------+
 *              |              ^
 *              v              |
 *         ingress (RX)   egress (TX)
 *              |              ^
 *              v              |
 *
 *              Linux Networking
 *
 * Packets transmitted on TUN1 will appear as received on TUN2 and
 * vice-versa.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

/*
 * Create a tun interface.
 *
 * Function code adapted from:
 * http://backreference.org/2010/03/26/tuntap-interface-tutorial/
 * Read the original post for a detailed documentation of the tun/tap
 * API.
 *
 * TUNSETIFF ioctl flags:
 *   IFF_TUN   - TUN device (no Ethernet headers)
 *   IFF_TAP   - TAP device
 *   IFF_NO_PI - Do not provide packet information. When this flag is
 *               specified, packets will be "pure" IP packets, with no
 *               added bytes. Otherwise 4 extra bytes are added to the
 *               beginning of the packet (2 flag bytes and 2 protocol
 *               bytes).
 *
 * See
 * http://backreference.org/2010/03/26/tuntap-interface-tutorial/#comment-24741
 * for an explanation regarding the (undocumented) 4 bytes header that
 * is controlled by IFF_NO_PI flag.
 */
int tun_alloc(char *dev)
{
	struct ifreq ifr;
	int fd;

	if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		perror("open");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	if (*dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);

	if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	strcpy(dev, ifr.ifr_name);
	return fd;
}

int main(int argc, char **argv)
{
	char dev1[IFNAMSIZ] = "tun%d";
	char dev2[IFNAMSIZ] = "tun%d";
	unsigned char buf[16384];
	int fd1, fd2;

	fd1 = tun_alloc(dev1);
	if (fd1 < 0)
		return 1;
	printf("Created: %s\n", dev1);

	fd2 = tun_alloc(dev2);
	if (fd2 < 0)
		return 1;
	printf("Created: %s\n", dev2);

	while (1) {
		fd_set fds;
		int nfds;
		ssize_t len;

		fflush(stdout);
		fflush(stderr);

		FD_ZERO(&fds);
		FD_SET(fd1, &fds);
		FD_SET(fd2, &fds);
		nfds = (fd2 > fd1 ? fd2 : fd1) + 1;

		nfds = select(nfds, &fds, 0, 0, 0);
		if (nfds < 0) {
			perror("select");
			continue;
		}

		if (FD_ISSET(fd1, &fds)) {
			len = read(fd1, buf, sizeof(buf));
			if (len < 0)
				perror("read");
			else {
				printf("read %d from fd %d\n", len, fd1);
				if (write(fd2, buf, len) < 0)
					perror("write");
			}
		}

		if (FD_ISSET(fd2, &fds)) {
			len = read(fd2, buf, sizeof(buf));
			if (len < 0)
				perror("read");
			else {
				printf("read %d from fd %d\n", len, fd2);
				if (write(fd1, buf, len) < 0)
					perror("write");
			}
		}
	}
	return 0;
}
