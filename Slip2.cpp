// slip2.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"

void pslip2(long irec,FILE *ptr,DB *df)
{
   ptr = topen("test.txt");
   PrintTicketSlip("slip2.rep",ptr,df);
   fclose(ptr);
   junction("test.txt",0);
	
}
