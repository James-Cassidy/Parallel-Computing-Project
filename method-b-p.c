/**
 * Method B
 * 
 * Load the source file into a single char array (one line)
 * Load the pattern lines (each of fixed length SEARCH_LINE_SIZE)
 * 
 * Search the source line looking for patterns, returning the number 
 * of instances of the pattern found (0 if not found)
 * 
 * For each pattern output a line to stdout of the format:
 * **,pattern,count\n
 * 
 * 
 */

#include <stdio.h>
#include <omp.h>    //Open MP
#include "search.h" // include the search library

int main(int ac, char** av)
{
    // get the sourcefile and pattern file as parameters when run or error if not present
    char *sourcefile,*patternfile;

    if (ac<2)
    {
        FatalError("Usage command sourcefile.txt patternfile.txt");
    }

    sourcefile=av[1];
    patternfile=av[2];


    // load the source data into chunks of SOURCE_LINE_SIZE
    struct FileData fd;
    readSourceFile(sourcefile,&fd,FILE_LOAD_ONELINE,1);

    // load the pattern file of fixed length lines
    struct FileData pat;
    readSourceFile(patternfile,&pat,FILE_LOAD_LINES,SEARCH_LINE_SIZE);

    // output some data
    #pragma omp parallel default(none) num_threads(1) shared(fd, pat)
    printf("Read %ld lines for %ld characters in source, %ld lines for %ld chars in pattern\n",fd.lines,fd.length,pat.lines,pat.length);
    printf("Loaded patterns for searching are:\n");

    #pragma omp parallel for default (none) shared (fd,pat) num_threads(16) schedule(auto)
    for (long p=0; p<pat.lines; ++p)
        printf("[%ld] %s\n",p,pat.data[p]);


    // array for the results of the pattern counting, initialise to zero
    long patterncount[pat.lines];

    #pragma omp parallel for default(none) shared(pat,fd) schedule(auto) lastprivate(patterncount)
    for(long i=0; i<pat.lines; ++i)
    {
        patterncount[i]=0;
    }

    // loop through the patterns
    #pragma omp parallel for default(none) schedule(auto) num_threads(16) shared(pat,fd,patterncount)
    for(long p=0; p<pat.lines; ++p)
    {
            // do a search for the pattern on the line using counter mode, incrementing the pattern counter
            // we only have one line in this mode fd.data[0]
            #pragma omp reduction(+:patterncount)
            patterncount[p] += Search(fd.data[0],fd.linesize,pat.data[p],pat.linesize,SEARCH_MODE_COUNT);
    }

    // now output the results
    #pragma omp parallel for shared(pat,patterncount,fd) num_threads(1)
    for (long p=0; p<pat.lines; ++p)
        printf("**,%s,%ld\n",pat.data[p],patterncount[p]);

    
    return 0;
}