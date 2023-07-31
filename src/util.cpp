#include "util.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void checkHostName(int hostname) {
  if (hostname == -1) {
    perror("gethostname");
    exit(1);
  }
}

void checkHostEntry(struct hostent *hostentry) {
  if (hostentry == NULL) {
    perror("gethostbyname");
    exit(1);
  }
}

void checkIPbuffer(char *IPbuffer) {
  if (NULL == IPbuffer) {
    perror("inet_ntoa");
    exit(1);
  }
}

std::string get_hostname() {
  char hostbuffer[256] = {0x00};
  int hostname = 0;

  hostname = gethostname(hostbuffer, sizeof(hostbuffer));
  checkHostName(hostname);

  auto hostname_str = std::string(hostbuffer);
  return hostname_str;
}

std::string get_ip(const std::string &hostname) {
  struct hostent *host_entry = nullptr;
  char *IPbuffer = nullptr;

  host_entry = gethostbyname(hostname.c_str());
  checkHostEntry(host_entry);

  IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));

  auto ip_str = std::string(IPbuffer);
  return ip_str;
}
