# ------------------------------------------------------------
# Makefile-cc: An example Makefile for CMPUT 379 (incomplete)
#
# Usage: make -f Makefile msh379  // compile program
#        make -f Makefile run      // run the program
#	      	 	     	      // Note: making this target may trigger
#				      // compiling the program	 
#        make -f Makefile tar      // create a 'tar.gz' archive of 'allFiles'
#        make -f Makefile scp  // send '$(target).tar.gz' by scp to 'scpDir'
#        make -f Makefile clean     // remove unneeded files
# ------------------------------------------------------------

target=		submit
allFiles=       Makefile starter.cc 
scpDir=		username@labmachine.cs.ualberta.ca:directory
# ------------------------------------------------------------


msh379:  msh379.cc
	g++ -Wall msh379.cc -o msh379

run:	msh379
	./msh379

clean:
	rm *~ out.* *.o