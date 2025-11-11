#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <fs/vfs.h>
#include <fs/fat12.h>
#include <mm/kheap.h>

#define LOG_MOD_NAME 	"VFS"
#define LOG_MOD_ENABLE  0
#include <log.h>

extern fs_type_t 		fat12_fs_type; // defined in fat12.c

/* start with 16 possible, then realloc */
#define INIT_MPOINTS 16

/* Some helpful macros to help reduce verbosity */

#define MOUNT(fs_type, src_dev) \
	( (fs_type)->vfs_ops.mount(src_dev) )

#define UNMOUNT(fs) \
	( (fs)->type->vfs_ops.unmount(fs) )

/* Private variables */

//! table of supported filesystem types
fs_type_t* fs_types[] = {
	&fat12_fs_type,
	NULL
};

//! table of mounted filesystems information
static  vfs** 		_mountpoints = NULL;
static 	size_t		_curr_size 	 = 0;

//! current root vnode of the VFS (will be deprecated)
static 	vnode* 		_vfs_root_vnode = NULL;

//! add a newly created entry to the list of mount points
static void _add_to_mpoints (vfs* _v) {

	for (size_t i = 0; i < _curr_size; i++)
	{
		if (!_mountpoints[i])
		{
			_mountpoints [i] = _v;
			return;
		}
	}

	void* temp = realloc (_mountpoints, _curr_size + 5);
	if (!temp)
	{
		LOG_DEBUG ("failed to reallocate memory for mountpoints\n");
		return;
	}

	_mountpoints = temp;
	_mountpoints [_curr_size] = _v;
	_curr_size += 5;

}


/* Public functions of the interface */

void vfs_init () {

	_vfs_root_vnode = NULL;
	_mountpoints 	= malloc (INIT_MPOINTS * sizeof(vfs*));
	memset (_mountpoints, 0, INIT_MPOINTS * sizeof (vfs*));
	_curr_size 		= 0;

}

int32_t vfs_mount (char* src_dev, char* mount_path, char* fs_type) {

	/* first find the corresponding fs type object from the available ones. */
	fs_type_t* 		fst = NULL;
	for (int i = 0; fs_types[i] != NULL; i++) {

		if (strcmp (fs_types[i]->fs_name, fs_type) == 0) {
			fst = fs_types[i];
			break;
		}
	
	}
	if (!fst) {
		LOG_ERROR ("filesystem type %s not supported\n", fs_type);
		return -1;
	}

	/* the fs driver first tries to read the superblock etc. on the specified
		device, by the mount function. */
	vfs* filesystem = MOUNT (fst, src_dev);
	if (!filesystem) {
		LOG_ERROR ("failed to mount filesystem %s from device %s\n",
					fs_type, src_dev);
		return -1;
	}

	/* create the vnode where this fsys is mounted at and initialize to all 0 */
	vnode*  mount_at = malloc (sizeof(vnode));
	memset (mount_at, 0, sizeof(vnode));

	/* 1 added to mnt path to skip the starting '/'. the vnode type will be a
		directory. all mountpoints are always dirs */
	
	mount_at->vfs_mountedhere = filesystem;
	mount_at->type 			  = V_DIRECTORY;
	strncpy (mount_at->name, mount_path+1, sizeof(mount_at->name));
	
	/* the vcovered param is set to be NULL by the fs driver, and the kernel sets
		the appropriate mapping */
	
	filesystem->vcovered = mount_at;
	_add_to_mpoints (filesystem);

	LOG_DEBUG ("mounted filesystem %s from device %s at %s\n",
				fs_type, src_dev, mount_path ? mount_path : "/");

	return 0;
}

//! OPEN the file at the specified path with the given flags
file_t* vfs_open (const char* path, uint32_t flags) {

	/* traverse the path to find the file */
	char*  token      = NULL;
	char*  next_token = NULL;

	/* get the first token, which should be the mountpoint name */
	token = strtok_r (path, "/", &next_token);

	vnode* root_node = NULL; 	// find from list of mounted fs

	for (size_t i = 0; i < _curr_size; i++)
	{
		if (!_mountpoints[i])
			continue;
		
		vfs* _mp = _mountpoints[i];
		if ( strcmp(_mp->vcovered->name, token) == 0 )
		{
			root_node = _mp->vroot;
			LOG_DEBUG ("switching to mounted fs %s at %s\n",
						_mp->type->fs_name, _mp->vcovered->name);
		}
	}

	if (!root_node) {
		LOG_ERROR ("no mounted filesystem found for token %s\n", token);
		return NULL;
	}

	// call the open function of the filesystem to open the file
	vnode* result = root_node->ops->open (root_node, next_token, flags);

	file_t* file 	= malloc (sizeof(file_t));
	file->vnode_ptr = result;
	file->f_offset  = 0;
	file->f_flags   = flags;

	return file;

}

//! read the file into a buffer
int32_t vfs_read (file_t* file, void* buf, uint32_t size) {

	LOG_DEBUG ("vfs_read: file=%p, buff=%p, size=%u\n", file, buf, size);

	if (!file || !file->vnode_ptr || !buf || size == 0) {
		LOG_ERROR ("invalid file or buffer specified for read\n");
		return -1;
	}

	/* read operation returns non-zero num bytes read */
	int32_t bytes_read = file->vnode_ptr->ops->read (file->vnode_ptr,
													 file->f_offset,
													 size, buf);
	if (bytes_read > 0) {
		file->f_offset += bytes_read;
	}

	return bytes_read;
	
}
