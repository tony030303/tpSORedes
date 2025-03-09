/* tcbref.c - tcbref, tcbunref */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcbref  -  increment the reference count of a TCB
 *------------------------------------------------------------------------
 */
void	tcbref(
	  struct tcb	*ptcb		/* Ptr to a TCB			*/
       )
{
	ptcb->tcb_ref++;
	return;
}

/*------------------------------------------------------------------------
 *  tcbunref  -  decrement the reference count of a TCB and free the
 *			TCB if the reference count reaches zero
 *------------------------------------------------------------------------
 */
void	tcbunref(
	  struct tcb	*ptcb		/* Ptr to a TCB			*/
       )
{
        int32 estado1, estado2;
        ptcb -> tcb_ref--; ///cambio
	if (ptcb->tcb_ref <= 0) {
	
		estado1 = freemem ((char *)ptcb->tcb_rbuf, ptcb->tcb_rbsize);
		estado2 = freemem ((char *)ptcb->tcb_sbuf, ptcb->tcb_sbsize);
		//se añade una condición para evitar que se coloque un TCB como libre si hubo algún error liberando la memoria de los buffers
		if(estado1 != SYSERR && estado2 != SYSERR){ 
		  kprintf("HABILITADO %d\n" ,ptcb);
		  ptcb->tcb_state = TCB_FREE;
		
		} 
		
	}
	return;
}
