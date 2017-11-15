// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

	static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// ProcessAddressSpace::ProcessAddressSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

ProcessAddressSpace::ProcessAddressSpace(OpenFile *executable)
{

	NoffHeader noffH;
	unsigned int i, size;
	unsigned vpn, offset;
	TranslationEntry *entry;
	unsigned int pageFrame;
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) && 
			(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
		+ UserStackSize;	// we need to increase the size
	// to leave room for the stack
	numVirtualPages = divRoundUp(size, PageSize);
	size = numVirtualPages * PageSize;
	backup_mem = new char[size];
	//ASSERT(numVirtualPages+numPagesAllocated <= NumPhysPages);		// check we're not trying
	// to run anything too big --
	// at least until we have
	// virtual memory

	DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
			numVirtualPages, size);
	// first, set up the translation 
	KernelPageTable = new TranslationEntry[numVirtualPages];
	for (i = 0; i < numVirtualPages; i++) {
		KernelPageTable[i].virtualPage = i;
		//KernelPageTable[i].physicalPage = i+numPagesAllocated; //No allocation of physical page
		KernelPageTable[i].physicalPage = -1;
		KernelPageTable[i].valid = FALSE;
		KernelPageTable[i].use = FALSE;
		KernelPageTable[i].dirty = FALSE;
		KernelPageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
		// a separate page, we could set its 
		// pages to be read-only
		KernelPageTable[i].shared = FALSE;
		KernelPageTable[i].backup = FALSE;
	}
	// zero out the entire address space, to zero the unitialized data segment 
	// and the stack segment
	//bzero(&machine->mainMemory[numPagesAllocated*PageSize], size);

	//numPagesAllocated += numVirtualPages;

	// then, copy in the code and data segments into memory
	/*
	 *    if (noffH.code.size > 0) {
	 *        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
	 *                        noffH.code.virtualAddr, noffH.code.size);
	 *        vpn = noffH.code.virtualAddr/PageSize;
	 *        offset = noffH.code.virtualAddr%PageSize;
	 *        entry = &KernelPageTable[vpn];
	 *        pageFrame = entry->physicalPage;
	 *        executable->ReadAt(&(machine->mainMemory[pageFrame * PageSize + offset]),
	 *                        noffH.code.size, noffH.code.inFileAddr);
	 *    }
	 *    if (noffH.initData.size > 0) {
	 *        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
	 *                        noffH.initData.virtualAddr, noffH.initData.size);
	 *        vpn = noffH.initData.virtualAddr/PageSize;
	 *        offset = noffH.initData.virtualAddr%PageSize;
	 *        entry = &KernelPageTable[vpn];
	 *        pageFrame = entry->physicalPage;
	 *        executable->ReadAt(&(machine->mainMemory[pageFrame * PageSize + offset]),
	 *                        noffH.initData.size, noffH.initData.inFileAddr);
	 *    }
	 *
	 */
}

//----------------------------------------------------------------------
// ProcessAddressSpace::ProcessAddressSpace (ProcessAddressSpace*) is called by a forked thread.
//      We need to duplicate the address space of the parent.
//----------------------------------------------------------------------

ProcessAddressSpace::ProcessAddressSpace(ProcessAddressSpace *parentSpace,int child_pid )
{
	NachOSThread *child_thread = threadArray[child_pid];
	numVirtualPages = parentSpace->GetNumPages();
	unsigned i, size = numVirtualPages * PageSize;
	//ASSERT(numVirtualPages+numPagesAllocated <= NumPhysPages);                // check we're not trying
	// to run anything too big --
	// at least until we have
	// virtual memory

	backup_mem = new char[size];

	DEBUG('a', "Initializing address space, num pages %d, size %d\n",
			numVirtualPages, size);
	// first, set up the translation
	TranslationEntry* parentPageTable = parentSpace->GetPageTable();
	KernelPageTable = new TranslationEntry[numVirtualPages];
	int curPages=0;
	for (i = 0; i < numVirtualPages; i++) {
		KernelPageTable[i].virtualPage = i;
		//KernelPageTable[i].physicalPage = i+numPagesAllocated;
		KernelPageTable[i].physicalPage = -1;
		KernelPageTable[i].valid = parentPageTable[i].valid;
		KernelPageTable[i].use = parentPageTable[i].use;
		KernelPageTable[i].dirty = parentPageTable[i].dirty;
		KernelPageTable[i].readOnly = parentPageTable[i].readOnly;  	// if the code segment was entirely on
		// a separate page, we could set its
		// pages to be read-only
		KernelPageTable[i].shared = parentPageTable[i].shared;
		KernelPageTable[i].backup = parentPageTable[i].backup;
		if (KernelPageTable[i].shared == TRUE ) {
			KernelPageTable[i].physicalPage= parentPageTable[i].physicalPage;
		}
		/*else{
		  KernelPageTable[i].physicalPage = curPages+numPagesAllocated;
		  curPages++;
		  }*/
	}

	// Copy the contents
	/*unsigned startAddrParent = parentPageTable[0].physicalPage*PageSize;
	  unsigned startAddrChild = numPagesAllocated*PageSize;
	  for (i=0; i<size; i++) {
	  machine->mainMemory[startAddrChild+i] = machine->mainMemory[startAddrParent+i];
	  }*/
	printf("START FORK LOOP\n");
	for (i = 0; i < numVirtualPages; i++){
		if(KernelPageTable[i].shared == TRUE ) {
			continue;
		}
		if(KernelPageTable[i].valid == FALSE)
		{
			if(KernelPageTable[i].backup == TRUE){
				// COPY PARENT BACKUP TO CHILD
				for(int j=0;j<PageSize;j++){
					backup_mem[i*PageSize+j] = parentSpace->backup_mem[i* PageSize + j ];
				}
			}
			continue;
		}
		int newPage;
		printf("ALLOCATING PAGE FROM FORK \n");
		if(!((machine->availablePages)->IsEmpty())){
		//if(numPagesAllocated < NumPhysPages){
			printf("TAKING from available\n");
			newPage = (int)((machine->availablePages)->Remove());
			numPagesAllocated++;
		}
		else{
			
			/*if(PageAlgo==1){
				do{
					newPage = Random()%NumPhysPages;
				}while(newPage==parentPageTable[i].physicalPage||pagetoShared[newPage]==TRUE);
			}
			else if(PageAlgo==2){
				//newPage = (int)FIFOlist->Remove();
			}*/
			newPage = getNewPage(parentPageTable[i].physicalPage);
			// DEBUG('a',"REPLACEMENT");
			int vpn_old = pagetoVPN[newPage];
			// DEBUG('a',"vpn_old:%d",vpn_old);
			printf(" FROM FORK vpn_old:%d newPage: %d PID:%d\n", vpn_old,newPage, pagetothread[newPage]->GetPID());
			NachOSThread *old_thread = pagetothread[newPage];
			TranslationEntry *old_table;
			if(old_thread->GetPID() != child_pid){
				old_table = (old_thread->space)->GetPageTable();
			}else{
				old_table = KernelPageTable;
			}
			old_table[vpn_old].valid = FALSE;
			if(old_table[vpn_old].dirty==TRUE){
				// DEBUG('a',"Backup");
				for(int j=0; j<PageSize;j++){
					(old_thread->space)->backup_mem[vpn_old*PageSize+j] = machine->mainMemory[newPage * PageSize + j ];
				}
				old_table[vpn_old].backup = TRUE;
			}
		}
		FIFO[newPage] = stats->totalTicks;
		LRU[newPage] = stats->totalTicks;
		LRUCLOCK[newPage] = 1;
		//	if(newPage!=parentPageTable[i].physicalPage && pagetoShared[newPage]==FALSE)
		//		FIFOlist->Append((void *)newPage);
		pagetoVPN[newPage] = i;
		pagetothread[newPage] = child_thread;
		KernelPageTable[i].physicalPage = newPage;
		curPages++;
		unsigned startAddrParent = parentPageTable[i].physicalPage*PageSize;
		unsigned startAddrChild = KernelPageTable[i].physicalPage*PageSize;
		for (unsigned j=0; j < PageSize; j++){
			machine->mainMemory[startAddrChild+j] = machine->mainMemory[startAddrParent+j];
		}
	}
	printf("END FORK LOOP\n");

	//numPagesAllocated += numVirtualPages;
	//numPagesAllocated += curPages;
}

//----------------------------------------------------------------------
// ProcessAddressSpace::~ProcessAddressSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

ProcessAddressSpace::~ProcessAddressSpace()
{
	printf("In destructor\n");
	for(int i=0;i<numVirtualPages;i++)
	{
		if(KernelPageTable[i].shared == TRUE || KernelPageTable[i].valid == FALSE)
			continue;
		// not a shared page
		// deallocate it
		int physPage = KernelPageTable[i].physicalPage;
		(machine->availablePages)->Append((void *)physPage);
	}
	delete KernelPageTable;
	delete backup_mem;
	printf("END destructor\n");
}

//----------------------------------------------------------------------
// ProcessAddressSpace::InitUserModeCPURegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

	void
ProcessAddressSpace::InitUserModeCPURegisters()
{
	int i;

	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, 0);

	// Initial program counter -- must be location of "Start"
	machine->WriteRegister(PCReg, 0);	

	// Need to also tell MIPS where next instruction is, because
	// of branch delay possibility
	machine->WriteRegister(NextPCReg, 4);

	// Set the stack register to the end of the address space, where we
	// allocated the stack; but subtract off a bit, to make sure we don't
	// accidentally reference off the end!
	machine->WriteRegister(StackReg, numVirtualPages * PageSize - 16);
	DEBUG('a', "Initializing stack register to %d\n", numVirtualPages * PageSize - 16);
}

//----------------------------------------------------------------------
// ProcessAddressSpace::SaveContextOnSwitch
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void ProcessAddressSpace::SaveContextOnSwitch() 
{}

//----------------------------------------------------------------------
// ProcessAddressSpace::RestoreContextOnSwitch
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void ProcessAddressSpace::RestoreContextOnSwitch() 
{
	machine->KernelPageTable = KernelPageTable;
	machine->KernelPageTableSize = numVirtualPages;
}

	unsigned
ProcessAddressSpace::GetNumPages()
{
	return numVirtualPages;
}

	TranslationEntry*
ProcessAddressSpace::GetPageTable()
{
	return KernelPageTable;
}

void 
ProcessAddressSpace::setKernelPageTable(TranslationEntry *ktable, unsigned int numV){
	KernelPageTable=ktable;
	numVirtualPages=numV;	
}

int getNewPage(int parentPage)
{
	int newPage;
	if(PageAlgo==1){
		do{
			printf("ELSE\n");
			newPage = Random()%NumPhysPages;
		}while(newPage == parentPage || pagetoShared[newPage]==TRUE);	
	}
	else if(PageAlgo==2){
		int mintime = 1e9;
		int minidx = -1;
		for(int i=0;i<NumPhysPages;i++)
		{
			//printf("------------------------------ %d FIFO: %d\n",i,FIFO[i]);
			if(pagetoShared[i] == TRUE)
				continue;
			if(i == parentPage) continue;
			if(FIFO[i] < mintime)
			{
				mintime = FIFO[i];
				minidx = i;
			}
		}		
		newPage = minidx;
		//	newPage = (int)FIFOlist->Remove();
	}
	else if(PageAlgo==3)
	{
		int mintime = 1e9;
		int minidx = -1;
		for(int i=0;i<NumPhysPages;i++)
		{
			if(pagetoShared[i] == TRUE)
				continue;
			if(i == parentPage) continue;
			if(LRU[i] < mintime)
			{
				mintime = LRU[i];
				minidx = i;
			}
		}		
		newPage = minidx;
	}
	else {
		//LRU CLOCK
		while(LRUptr == parentPage || pagetoShared[LRUptr] == TRUE || LRUCLOCK[LRUptr] == 1){
			LRUCLOCK[LRUptr] = 0;
			LRUptr = (LRUptr+1)%NumPhysPages;
		}
		newPage = LRUptr;
	}
	return newPage;
}
void
ProcessAddressSpace::handlePageFault(int vpn){
	int newPage;
	printf("ALLOCATING PAGE FROM HANDLE\n");
	printf("FAULT: %d\n",numPagesAllocated);
	if(!((machine->availablePages)->IsEmpty())){
	//if(numPagesAllocated < NumPhysPages){
		printf("IF\n");
		newPage = (int)((machine->availablePages)->Remove());
		numPagesAllocated++;
	}
	else {
		
		   /*if(PageAlgo==1){
			   do{
				   printf("ELSE\n");
				   newPage = Random()%NumPhysPages;
			   }while(pagetoShared[newPage]==TRUE);	
		   }
		   else if(PageAlgo==2){
			   //newPage = (int)FIFOlist->Remove();
		   }*/
		newPage = getNewPage(-1);
		// DEBUG('a',"REPLACEMENT");
		int vpn_old = pagetoVPN[newPage];
		// DEBUG('a',"vpn_old:%d",vpn_old);
		printf("vpn_old:%d newpage: %d\n", vpn_old, newPage);
		NachOSThread *old_thread = pagetothread[newPage];
		printf("vpn_old:%d PID: %d\n", vpn_old,old_thread->GetPID());
		printf("vpn_old:%d newPID: %d\n", vpn_old,currentThread->GetPID());
		
		TranslationEntry *old_table = (old_thread->space)->GetPageTable();
		old_table[vpn_old].valid = FALSE;
		if(old_table[vpn_old].dirty==TRUE){
			// DEBUG('a',"Backup");
			for(int j=0; j<PageSize;j++){
				//printf("char bit:%d\n", j);
				//printf("PID of old: %d\n",old_thread->GetPID());
				//printf("size of old backup: %d\n",sizeof((old_thread->space)->backup_mem));
				(old_thread->space)->backup_mem[vpn_old*PageSize+j] = machine->mainMemory[newPage * PageSize + j ];
				//printf("char bit:%d\n", j);
			}
			old_table[vpn_old].backup = TRUE;
		}
	}
	FIFO[newPage] = stats->totalTicks;
	LRU[newPage] = stats->totalTicks;
	LRUCLOCK[newPage] = 1;
	printf("newPage:%d vpn: %d\n", newPage,vpn);
	/*
	   if(pagetoShared[newPage]==FALSE){
	   FIFOlist->Append((void *)newPage);
	   }*/
	// DEBUG('a',"newPage:%d", newPage);
	pagetoVPN[newPage] = vpn;
	pagetothread[newPage] = currentThread;
	machine->KernelPageTable[vpn].physicalPage = newPage;
	machine->KernelPageTable[vpn].valid = TRUE;
	machine->KernelPageTable[vpn].dirty = FALSE;
	if(machine->KernelPageTable[vpn].backup==FALSE){
		printf("executable\n");
		//TODO need to read from executable
		// OpenFile *executable = fileSystem->Open(execFile);
		if (executableVar == NULL) {
			printf("Empty file\n");
			ASSERT(false);
		}
		NoffHeader noffH;
		executableVar->ReadAt((char *)&noffH, sizeof(noffH), 0);
		if ((noffH.noffMagic != NOFFMAGIC) && 
				(WordToHost(noffH.noffMagic) == NOFFMAGIC))
			SwapHeader(&noffH);
		ASSERT(noffH.noffMagic == NOFFMAGIC);
		bzero(&machine->mainMemory[newPage*PageSize], PageSize);

		executableVar->ReadAt(&(machine->mainMemory[newPage * PageSize ]),PageSize, noffH.code.inFileAddr + vpn*PageSize);
	}
	else{
		printf("backup\n");
		//READ from backup
		for(int i=0;i<PageSize;i++){
			machine->mainMemory[newPage * PageSize + i ] = (currentThread->space)->backup_mem[vpn*PageSize+i];
		}
	}
	printf("DONE\n");
}

