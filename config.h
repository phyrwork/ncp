#ifndef NCP_CONFIG_H
#define NCP_CONFIG_H

#define NUM_PORTS_DEFAULT 4
#define NUM_PORTS_MAX 32
#define BASE_PORT_DEFAULT 8024

#define SIZEOF_NCP_RESPONSE(num_ports) (sizeof(ncp_opt_t) + (num_ports-1)*sizeof(unsigned short))

#endif /* NCP_CONFIG_H */
