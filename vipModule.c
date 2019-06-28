

///////////// Do random without repeatition /////////////////////////////////////////
int var[Maxrandom];
for (int i = 0; i < Maxrandom; i++) {     // fill array
    var[i] = i;
	}

	for (int i = 0; i < Maxrandom; i++) {    // shuffle array to make it unique
    int temp = var[i];
    int randomIndex = rand() % Maxrandom;

    var[i]   = var[randomIndex];
    var[randomIndex] = temp;

	}

/// the same thing from exam template <trainProcons.c>
        // asssigning train a unique station id
            int  *vet,j,k;
            for (int i=0;i<4;i++)
            vet[i] = i; 

            k = 4;
            for (int i=0;i<2;i++) {
                j = rand() % k;
                train[i].stationId = vet[j] % 4;  
                vet[j] = vet[k-1];
                k--;
          } 
        //end assignment;
///////////// End Do random without repeatition /////////////////////////////////////////    

////////////////directories exam template(read directory, check file or dir, lock file and write)/////////////////////////////////////////////////////////////////

        	//////////check if its directory or not////////////////////
                        if (stat(argv[1], &stat_buf) < 0)
                        perror("stat error");
                        if ((stat_buf.st_mode & S_IFMT) != S_IFDIR)
                        perror("not a directory");		
        	//////////end check dir////////////////////////////////////
                    _POSIX_PATH_MAX            // it gets the maximum length of a path for linux file system
                     DIR    *dp;               //  directory descriptor
                    struct dirent *dirp;       // read directory
                    struct flock lock;        ///for locking   
                  

////////////////end directories////////////////////////////////////////////////////////
