#!/usr/bin/perl
use strict;
use warnings;

while (<>)
 {
 if (m/(.*) rise ([0-9]*) set ([0-9]*)/)
	 {
		 printf ("%s rise %d:%02d set %d:%02d\n", $1, $2/60, $2 % 60, $3/60, $3 % 60);
	 }
 else
	 {
		 if (m/(.*) set mins ([0-9]*)/)
			 {
				 printf ("%s set mins %d:%02d\n", $1, $2/60, $2 % 60);
			 }
		 else
			 {
				 if (m/(.*) rise mins ([0-9]*)/)
					 {
						 printf ("%s rise mins %d:%02d\n", $1, $2/60, $2 % 60);
					 }
				 else
					 {
						 if (m/(.*) rise mins ([0-9]*)/)
							 {
								 printf ("%s rise mins %d:%02d\n", $1, $2/60, $2 % 60);
							 }
						 else
							 {
								 print;
							 }
					 }
			 }
	 }
}
