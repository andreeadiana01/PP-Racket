#include "skel.h"
#include <stdio.h>

struct route_table_entry *route_table;
int rtable_size;

struct arp_entry *arp_table;
int arp_table_size;

void parse_arp_table() 
{
	FILE *f;
	f = fopen("arp_table.txt", "r");
	DIE(f == NULL, "Failed to open arp_table.txt");
	char line[100];
	int i = 0;
	for (i = 0; fgets(line, sizeof(line), f); i++) {
		char ip_str[50], mac_str[50];
		sscanf(line, "%s %s", ip_str, mac_str);
		arp_table[i].ip = inet_addr(ip_str);
		int rc = hwaddr_aton(mac_str, arp_table[i].mac);
		DIE(rc < 0, "invalid MAC");
	}
	arp_table_size = i;
	fclose(f);
}

void parse_rtable()
{
	FILE *f;
	f = fopen("rtable.txt", "r");
	DIE(f == NULL, "Failed to open rtable.txt");
	char line[100];
	int i = 0;
	for (i = 0; fgets(line, sizeof(line), f); i++) {
		char prefix_str[25], next_hop_str[25], mask_str[25], interface_str[25];
		sscanf(line, "%s %s %s %s ", prefix_str, next_hop_str, mask_str, interface_str);
		route_table[i].prefix = inet_addr(prefix_str);
		route_table[i].next_hop = inet_addr(next_hop_str);
		route_table[i].mask = inet_addr(mask_str);
		route_table[i].interface = atoi(interface_str);
	}
	rtable_size = i;
	fclose(f);
}

struct route_table_entry* search_for_best_route(int left, int right, __u32 searched, int acc)
{
	if (left <= right) {
		int middle = left + (right - left) / 2;
		if ((searched & route_table[middle].mask) >= (route_table[middle].prefix)) {
			return search_for_best_route(middle + 1, right, searched, middle);
		} else {
			return search_for_best_route(left, middle - 1, searched, acc);
		}
	}
	if (acc == -1) {
			return NULL;
		}
	return &route_table[acc];
}

int compare(const struct route_table_entry *r1, const struct route_table_entry *r2) {
	if (r1->prefix == r2->prefix){
		return (r1->mask > r2->mask);
	}
	return r1->prefix > r2->prefix;
}

struct arp_entry *get_arp_entry(__u32 ip) {
	for (int i = 0 ; i < arp_table_size ; ++i) {
		if (arp_table[i].ip == ip) {
			return &arp_table[i];
		}
	}
    return NULL;
}


int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);
	packet m;
	int rc;
	rtable_size = 0;
	arp_table_size = 0;

	init();
	route_table = malloc(sizeof(struct route_table_entry) * 100000);
	arp_table = malloc(sizeof(struct  arp_entry) * 10000);
	DIE(route_table == NULL, "memory");
	DIE(arp_table == NULL, "memory");

	parse_arp_table();
	parse_rtable();

	arp_table = realloc(arp_table, sizeof(struct  arp_entry) * arp_table_size);
	route_table = realloc(route_table, sizeof(struct route_table_entry) * rtable_size);

	qsort(route_table, rtable_size, sizeof(struct route_table_entry), compare);

	while (1) {

		rc = get_packet(&m);
		DIE(rc < 0, "get_message");
		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		int old_check_sum = ip_hdr->check;
		ip_hdr->check = 0;

		if (old_check_sum != ip_checksum(ip_hdr, sizeof(struct iphdr))) {
			fprintf(stderr, "Wrong checksum!\n");
			continue;
		}

		if (ip_hdr->ttl <= 1) {
			fprintf(stderr, "Package dead!\n");
			continue;
		}

		struct route_table_entry *route = search_for_best_route(0, rtable_size - 1, ip_hdr->daddr, -1);

		if (route == NULL) {
			fprintf(stderr, "Route not found!\n");
			continue;
		}

		ip_hdr->ttl--;
		ip_hdr->check = 0;
		ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));

		struct arp_entry *entry = get_arp_entry(route->next_hop);

		if (entry == NULL) {
			fprintf(stderr, "No arp entry!\n");
			continue;
		}

		get_interface_mac(route->interface, eth_hdr->ether_shost);
		send_packet(route->interface, &m);
	}
}