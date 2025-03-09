/* tcp_in.c  -  tcp_in */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcp_in  -  process an incoming TCP segment
 *------------------------------------------------------------------------
 */
void	tcp_in(
	  struct netpacket *pkt		/* Ptr to arriving packet	*/
	)
{
	int32	i;			/* Iterates through TCBs	*/
	int32	partial;		/* -1 or slot of partial match	*/
	int32	complete;		/* -1 or slot of full match	*/
	byte	ebcast[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	int32	entry;
	uint16	len;			/* Length of the segment	*/
	struct	tcb	*tcbptr;	/* Ptr to TCB entry		*/
        
        //kprintf("tcp_in: parada 1\n");
	/* Get pointers to Ethernet header, IP header and TCP header */

	len = pkt->net_iplen;

	/* Reject broadcast or multicast packets out of hand */

	if ((memcmp(pkt->net_ethdst,ebcast,ETH_ADDR_LEN)== 0) ||
				(pkt->net_ipdst == IP_BCAST)) {
		freebuf ((char *)pkt);
		return;
	}
	//kprintf("tcp_in: parada 2\n");
/*DEBUG*/ //kprintf("\nIN: seq %x, ackseq %x\n", pkt->net_tcpseq, pkt->net_tcpack);
	//pdumph(pkt);

	/* Validate header lengths */

	if (len < (IP_HDR_LEN + TCP_HLEN(pkt)) ) {
		freebuf ((char *)pkt);
		return;
	}
	//kprintf("tcp_in: parada 3\n");

	partial = complete = -1;

	/* Insure exclusive use */

	wait (Tcp.tcpmutex);
	/*DEBUG*/ //kprintf("IN: ipsrc %08x, ipdst %08x, sport %u, dport %u\n",
  //  pkt->net_ipsrc, pkt->net_ipdst, pkt->net_tcpsport, pkt->net_tcpdport);

	for (i = 0; i < Ntcp; i++) {
	        tcbptr = &tcbtab[i];
		 /*DEBUG*/ /*kprintf("TCB[%d]: state %d, lip %08x, rip %08x, lport %u, rport %u\n",
       i, tcbptr->tcb_state, tcbptr->tcb_lip, tcbptr->tcb_rip, tcbptr->tcb_lport, tcbptr->tcb_rport);*/

		if (tcbptr->tcb_state == TCB_FREE) {

			/* Skip unused emtries */
			continue;
		} else if (tcbptr->tcb_state == TCB_LISTEN) {

			/* Check partial match */

			/* Accept only if the current entry	*/
			/* matches and is more specific		*/

			if (((tcbptr->tcb_lip == 0
			      && partial == -1)
			     || pkt->net_ipdst == tcbptr->tcb_lip)
			    && pkt->net_tcpdport == tcbptr->tcb_lport) {
				partial = i;
			}
			continue;
		} else {

			/* Check for exact match */

			if (pkt->net_ipsrc == tcbtab[i].tcb_rip
			    && pkt->net_ipdst == tcbptr->tcb_lip
			    && pkt->net_tcpsport == tcbptr->tcb_rport
			    && pkt->net_tcpdport == tcbptr->tcb_lport) {
				complete = i; //complete = i
				break;
			}
			continue;
		}
	}
	//kprintf("tcp_in: parada 4\n");
        //kprintf("complete = %d , partial = %d\n" , complete, partial);
	/* See if full match found, partial match found, or none */

	if (complete != -1) {
		entry = complete;
	} else if (partial != -1) {
		entry = partial;
	} else {
                //tercera ejecución entra acà
		/* No match - send a reset and drop the packet */
                //kprintf("NOMATCH!\n");
		signal (Tcp.tcpmutex);
		//kprintf("tcp_in: entro tcpreset\n");
		tcpreset (pkt);
		//kprintf("SUPERA2\n");
		//kprintf("tcp_in: salgo tcpreset\n");
		freebuf ((char *)pkt);
		//kprintf("FREBU!\n");
		
		return;
	}

	/* Wait on mutex for TCB and release global mutex */
        //kprintf("tcp_in:acceso tcb_mutex %d\n" , entry);
	wait (tcbtab[entry].tcb_mutex);
	signal (Tcp.tcpmutex);
        //kprintf("tcp_in: paso de acá\n");
	/* Process the segment according to the state of the TCB */

	//kprintf("tcp_in: entro a tcpdisp\n");
	tcpdisp (&tcbtab[entry], pkt);
	

	
	
	
	//kprintf("tcp_in: salgo de tcpdisp\n");
	signal (tcbtab[entry].tcb_mutex);
	freebuf ((char *)pkt);

	return;
}
