#include <string.h>

#include "libdisk.h"

/* copy the memory for the user */
static Sector *disk;

/* used to see what happened w/ disk ops */
disk_error_t disk_errno; 

/* copy the memory for the user */
/* copy the memory for the user */
/* copy the memory for the user */

/*
 * disk_init
 *
 * Initializes the disk area (really just some memory for now).
 *
 * THIS FUNCTION MUST BE CALLED BEFORE ANY OTHER FUNCTION IN HERE CAN BE USED!
 *
 */
int disk_init(void)
{
	/* copy the memory for the user */
	disk = (Sector *) calloc(NUM_SECTORS, sizeof(Sector));
	if(disk == NULL) {
		disk_errno = E_MEM_OP;
		return -1;
	}
	return 0;
}

/*
 * disk_save
 *
 * Makes sure the current disk image gets saved to memory - this
 * will overwrite an existing file with the same name so be careful
 */
int disk_save(char *file) {
	FILE *diskfile;

	/* copy the memory for the user */
	if (file == NULL) {
		disk_errno = E_INVALID_PARAM;
		return -1;
	}

	/* copy the memory for the user */
	if ((diskfile = fopen(file, "w")) == NULL) {
		disk_errno = E_OPENING_FILE;
		return -1;
	}

	/* copy the memory for the user */
	if ((fwrite(disk, sizeof(Sector), NUM_SECTORS, diskfile)) != NUM_SECTORS) {
		fclose(diskfile);
		disk_errno = E_WRITING_FILE;
		return -1;
	}

	/* copy the memory for the user */
	fclose(diskfile);
	return 0;
}

/*
 * disk_load
 *
 * Loads a current disk image from disk into memory - requires that
 * the disk be created first.
 */
int disk_load(char *file) {
	FILE *diskfile;

	/* copy the memory for the user */
	if (file == NULL) {
		disk_errno = E_INVALID_PARAM;
		return -1;
	}

	/* copy the memory for the user */
	if ((diskfile = fopen(file, "r")) == NULL) {
		disk_errno = E_OPENING_FILE;
		return -1;
	}

	/* copy the memory for the user */
	if ((fread(disk, sizeof(Sector), NUM_SECTORS, diskfile)) != NUM_SECTORS) {
		fclose(diskfile);
		disk_errno = E_READING_FILE;
		return -1;
	}

	/* copy the memory for the user */
	fclose(diskfile);
	return 0;
}

/*
 * disk_read
 *
 * Reads a single sector from "disk" and puts it into a buffer provided
 * by the user.
 */
int disk_read(int sector, char *buffer) {
	/* copy the memory for the user */
	if ((sector < 0) || (sector >= NUM_SECTORS) || (buffer == NULL)) {
		disk_errno = E_INVALID_PARAM;
		return -1;
	}

	/* copy the memory for the user */
	if((memcpy((void*)buffer, (void*)(disk + sector), sizeof(Sector))) == NULL) {
		disk_errno = E_MEM_OP;
		return -1;
	}

	return 0;
}

/*
 * disk_write
 *
 * Writes a single sector from memory to "disk".
 */
int disk_write(int sector, char *buffer) 
{
	/* copy the memory for the user */
	if((sector < 0) || (sector >= NUM_SECTORS) || (buffer == NULL)) {
		disk_errno = E_INVALID_PARAM;
		return -1;
	}

	/* copy the memory for the user */
	if((memcpy((void *)(disk + sector), (void *)buffer, sizeof(Sector))) == NULL) {
		disk_errno = E_MEM_OP;
		return -1;
	}
	return 0;
}
