#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef DEBUG
#define myprintf(fmt,...) printf(fmt,##__VA_ARGS__)
#else
#define myprintf(fmt,...)
#endif
void recursive_remove(char* path)
{
    DIR*dir=opendir(path);
    if(dir==NULL)
    {
        myprintf("opendir %s failed\n",path);
        perror("opendir");
        return;
    }
    struct dirent*entry;
    while((entry=readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)
        {
            continue;
        }
        char newpath[1024];
        strcpy(newpath,path);
        strcat(newpath,"/");
        strcat(newpath,entry->d_name);
        if(entry->d_type==DT_DIR)
        {
            recursive_remove(newpath);
        }
        else
        {
            int temp=unlink(newpath);
            if(temp==-1)
            {
                myprintf("unlink %s failed\n",newpath);
                perror("unlink");
            }
        }
    }
    closedir(dir);
    int temp=rmdir(path);
    if(temp==-1)
    {
        myprintf("rmdir %s failed\n",path);
        perror("rmdir");
    }  
}
void rsync(char* src,char* dst)
{
    DIR*dir=opendir(src);
    if(dir==NULL)
    {
        myprintf("opendir %s failed\n",src);
        perror("opendir");
        return;
    }
    struct dirent*entry;
    struct dirent srcentries[1024];
    int i=0;
    while((entry=readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)
        {
            continue;
        }
        memcpy(&srcentries[i++],entry,sizeof(struct dirent));
    }
    int num_src_entries = i;
    closedir(dir);
    dir = opendir(dst);
    if(dir==NULL)
    {
        myprintf("opendir %s failed\n",dst);
        perror("opendir");
        return;
    }
    struct dirent dstentries[1024];
    i=0;
    while((entry=readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)
        {
            continue;
        }
        memcpy(&dstentries[i++],entry,sizeof(struct dirent));
    }
    closedir(dir);
    int num_dst_entries = i;
    for(int i=0;i<num_src_entries;i++)
    {
        myprintf("src:%s\n",srcentries[i].d_name);
    }
    for(int i=0;i<num_dst_entries;i++)
    {
        myprintf("dst:%s\n",dstentries[i].d_name);
    }
    for(int i=0;i<num_src_entries;i++)
    {
        int index=-1;
        if(srcentries[i].d_type==DT_DIR)
        {
            int index=-1;
            for(int j=0;j<num_dst_entries;j++)
            {
                if(strcmp(srcentries[i].d_name,dstentries[j].d_name)==0&&dstentries[j].d_type==DT_DIR)
                {
                    index=j;
                    break;
                }
            }
            if(index==-1)
            {
                char*currentdir = getcwd(NULL,0);
                chdir(dst);
                mkdir(srcentries[i].d_name,0777);
                chdir(currentdir);
                free(currentdir);
            }
            char newsrc[1024];
            char newdst[1024];
            strcpy(newsrc,src);
            strcat(newsrc,"/");
            strcat(newsrc,srcentries[i].d_name);
            strcpy(newdst,dst);
            strcat(newdst,"/");
            strcat(newdst,srcentries[i].d_name);
            rsync(newsrc,newdst);
            continue;
        }
        for(int j=0;j<num_dst_entries;j++)
        {
            if(strcmp(srcentries[i].d_name,dstentries[j].d_name)==0)
            {
                index=j;
                break;
            }
        }
        int created = 0;
        if(index==-1)
        {
            char* currentdir = getcwd(NULL,0);
            chdir(dst);
            int fd = open(srcentries[i].d_name,O_RDWR|O_CREAT,0666);
            if(fd==-1)
            {
                myprintf("open %s failed\n",srcentries[i].d_name);
                perror("open");
                return;
            }
            close(fd);
            created = 1;
            chdir(currentdir);
            free(currentdir);

        }
        struct stat srcstat,dststat;
        char srcfile[1024],dstfile[1024];
        strcpy(srcfile,src);
        strcat(srcfile,"/");
        strcat(srcfile,srcentries[i].d_name);
        strcpy(dstfile,dst);
        strcat(dstfile,"/");
        strcat(dstfile,srcentries[i].d_name);
        stat(srcfile,&srcstat);
        stat(dstfile,&dststat);
        if(srcstat.st_mtime==dststat.st_mtime&&srcstat.st_size==dststat.st_size&&srcstat.st_mode==dststat.st_mode&&srcstat.st_uid==dststat.st_uid&&srcstat.st_gid==dststat.st_gid)
        {
            continue;
        }
        if(created)
        {
            printf("[+] %s\n",dstfile);
        }
        else
        {
            printf("[o] %s\n",dstfile);
            int temp=unlink(dstfile);
            if(temp==-1)
            {
                myprintf("unlink %s failed\n",dstfile);
                perror("unlink");
            }
        }
        int fd1 = open(srcfile,O_RDONLY);
        if(fd1==-1)
        {
            myprintf("open %s failed\n",srcfile);
            perror("open");
            return;
        }
        int fd2 = open(dstfile,O_RDWR|O_CREAT,srcstat.st_mode);
        if(fd2==-1)
        {
            myprintf("open %s failed\n",dstfile);
            perror("open");
            return;
        }
        char buf[1024];
        int n;
        while((n=read(fd1,buf,1024))>0)
        {
            write(fd2,buf,n);
        }
        close(fd1);
        close(fd2);
        //Update the timestamp
        struct utimbuf times;
        times.actime = srcstat.st_atime;
        times.modtime = srcstat.st_mtime;
        utime(dstfile,&times);
        printf("[t] %s\n",dstfile);
    }
    for(int i=0;i<num_dst_entries;i++)
    {
        int index=-1;
        for(int j=0;j<num_src_entries;j++)
        {
            if(strcmp(dstentries[i].d_name,srcentries[j].d_name)==0&&srcentries[j].d_type==dstentries[i].d_type)
            {
                index=j;
                break;
            }
        }
        if(index!=-1)
            continue;
        char dstfile[1024];
        strcpy(dstfile,dst);
        strcat(dstfile,"/");
        strcat(dstfile,dstentries[i].d_name);
        if(dstentries[i].d_type==DT_DIR)
        {
            recursive_remove(dstfile);
            printf("[-] %s\n",dstfile);
        }
        else
        {
            int temp=unlink(dstfile);
            if(temp==-1)
            {
                myprintf("unlink %s failed\n",dstfile);
                perror("unlink");
            }
            printf("[-] %s\n",dstfile);
        }
        
    }
    //Updating permissions of files and directories
    for(int i=0;i<num_src_entries;i++)
    {
        char srcfile[1024];
        strcpy(srcfile,src);
        strcat(srcfile,"/");
        strcat(srcfile,srcentries[i].d_name);
        struct stat srcstat;
        stat(srcfile,&srcstat);
        char dstfile[1024];
        strcpy(dstfile,dst);
        strcat(dstfile,"/");
        strcat(dstfile,srcentries[i].d_name);
        struct stat dststat;
        stat(dstfile,&dststat);
        if(srcstat.st_mode!=dststat.st_mode)
        {
            chmod(dstfile,srcstat.st_mode);
            printf("[p] %s\n",dstfile);
        }
    }
    //Updating ownership of files and directories
    for(int i=0;i<num_src_entries;i++)
    {
        char srcfile[1024];
        strcpy(srcfile,src);
        strcat(srcfile,"/");
        strcat(srcfile,srcentries[i].d_name);
        struct stat srcstat;
        stat(srcfile,&srcstat);
        char dstfile[1024];
        strcpy(dstfile,dst);
        strcat(dstfile,"/");
        strcat(dstfile,srcentries[i].d_name);
        struct stat dststat;
        stat(dstfile,&dststat);
        if(srcstat.st_uid!=dststat.st_uid||srcstat.st_gid!=dststat.st_gid)
        {
            chown(dstfile,srcstat.st_uid,srcstat.st_gid);
            printf("[p] %s\n",dstfile);
        }
    }
    //Updating timestamps of directories
    for(int i=0;i<num_src_entries;i++)
    {
        char srcfile[1024];
        strcpy(srcfile,src);
        strcat(srcfile,"/");
        strcat(srcfile,srcentries[i].d_name);
        struct stat srcstat;
        stat(srcfile,&srcstat);
        char dstfile[1024];
        strcpy(dstfile,dst);
        strcat(dstfile,"/");
        strcat(dstfile,srcentries[i].d_name);
        struct stat dststat;
        stat(dstfile,&dststat);
        if(srcstat.st_mtime!=dststat.st_mtime)
        {
            struct utimbuf times;
            times.actime = srcstat.st_atime;
            times.modtime = srcstat.st_mtime;
            utime(dstfile,&times);
            printf("[t] %s\n",dstfile);
        }
    }

}
int main(int argc,char*argv[])
{
    if(argc!=3)
    {
        printf("Usage:%s <src> <dest>\n",argv[0]);
        exit(1);
    }
    char src[1024],dest[1024];
    if(argv[1][0]=='/')
    {
        strcpy(src,argv[1]);
    }
    else
    {
        char*home=getcwd(NULL,0);
        strcpy(src,home);
        strcat(src,argv[1]+1);
    }
    if(argv[2][0]=='/')
    {
        strcpy(dest,argv[2]);
    }
    else
    {
        char*home=getcwd(NULL,0);
        strcpy(dest,home);
        strcat(dest,argv[2]+1);
    }
    myprintf("src:%s,dest:%s\n",src,dest);
    rsync(src,dest);
}
