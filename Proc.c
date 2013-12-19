#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

typedef void (*ProcFunc)(void*); 

typedef struct PROC_CONTEXT
{
	int nSize;
	int pipefd[2];
	int nOldPipeFd[2];
} PROC_CONTEXT;

void StartProc( ProcFunc pFunc, void* pContext )
{
	pid_t procId = fork();	
	if ( procId > 0) 
		return;

	(*pFunc)(pContext);
	exit(0);
}

void NewProc(void* Context )
{
	PROC_CONTEXT* pContext = (PROC_CONTEXT*)Context;
	pContext->nSize--;
	
	// Parvia proces nqma OldFd zatova ne go zatvarq
	if ( pContext->nSize != 4 )
	{
		close( pContext->nOldPipeFd[0]);
		close( pContext->nOldPipeFd[1]);
	}

	memcpy( pContext->nOldPipeFd, pContext->pipefd, sizeof(pContext->nOldPipeFd) );
	
	close( pContext->nOldPipeFd[0] );
	pid_t* pidArray = NULL;

	int i = 0;
	// Последния процес не очаква да получи нищо
	if ( pContext->nSize != 1 )
	{
		pipe( pContext->pipefd );
	
		StartProc( &NewProc, (void*)pContext );

		close( pContext->pipefd[1] );
		pidArray = (pid_t*)malloc( sizeof(pid_t)*pContext->nSize);
	
		// Прочитане ид-то на всики без на нашия
		read ( pContext->pipefd[0], pidArray, sizeof(pidArray[0])*pContext->nSize - 1 );

	}
	else
	{
		pidArray = (pid_t*)malloc( sizeof(pid_t)*pContext->nSize);
	}

	printf( "My pId: %d!\n", getpid() );
	pidArray[ pContext->nSize - 1 ] = getpid();

	write( pContext->nOldPipeFd[1], pidArray, sizeof(pidArray[0])*pContext->nSize );

	free( pidArray);
}

int main()
{  
	PROC_CONTEXT recContext;
	recContext.nSize = 5;
	
	pipe( recContext.pipefd );
	
	StartProc( &NewProc, (void*)&recContext );

	close( recContext.pipefd[1] );
	
	int i = 0;
	pid_t pidArray[recContext.nSize ];
			
	// Прочитане ид-то на всики без на нашия
	read ( recContext.pipefd[0], pidArray, sizeof(pidArray) - sizeof(pidArray[0]) );
	printf( "My Main pId: %d!\n", getpid() );
	
	pidArray[ recContext.nSize - 1 ] = getpid();

	for ( i = 0; i < recContext.nSize; i++ )
		printf( "Proc: %d\n", pidArray[i] );
	
	return;
}

