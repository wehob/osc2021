#include "vfs.h"
#include "allocator.h"
#include "error.h"
#include "uart.h"

static mount my_mount;
static filesystem my_filesystem;

const char* slashIgnore(const char* src,char* dst,int size){
	for(int i=0;i<size;++i){
		if(src[i]==0){
			dst[i]=0;
			return 0;
		}else if(src[i]=='/'){
			dst[i]=0;
			return src+i+1;
		}else{
			dst[i]=src[i];
		}
	}
	ERROR("slashIgnore error!!");
	return 0;
}

file* vfs_open(const char* pathname,int flags){
	vnode* dir=my_mount.root;
	vnode* child;
	while(1){
		char prefix[PREFIX_LEN];
		pathname=slashIgnore(pathname,prefix,PREFIX_LEN);
		int idx=dir->v_ops->lookup(dir,&child,prefix);

		if(pathname){
			if(idx>=0){
				dir=child;
			}else{
				ERROR("invalid directory!!");
			}
		}else{
			if(idx>=0){
				break;//already exist
			}else{
				if((flags&O_CREAT)==0)ERROR("invalid file!!");
				dir->v_ops->create(dir,&child,prefix);
				break;
			}
		}
	}

	file* ret=(file*)dalloc(sizeof(file));
	ret->node=child;
	ret->f_pos=0;
	ret->f_ops=my_mount.root->f_ops;
	ret->flags=flags;
	return ret;
}

int vfs_close(file* f){
	dfree((unsigned long)f);
	return 0;
}

int vfs_write(file* f,const void* buf,unsigned long len){
	return f->f_ops->write(f,buf,len);
}

int vfs_read(file* f,void* buf,unsigned long len){
	return f->f_ops->read(f,buf,len);
}

void vfs_sync(file* f){
	f->f_ops->sync(f);
}

void vfs_init(void* setup_mount_f){
	int (*setup_mount)(filesystem*,mount*)=setup_mount_f;

	setup_mount(&my_filesystem,&my_mount);
	uart_printf("%s have been setup.\n\n",my_filesystem.name);
}
