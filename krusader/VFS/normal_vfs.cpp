/***************************************************************************
                       normal_vfs.cpp                             
                   -------------------
    copyright            : (C) 2000 by Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------

 ***************************************************************************

  A 

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifdef _OS_SOLARIS_
#include <strings.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
// QT includes
#include <qdir.h>
#include <qtimer.h>
// KDE includes
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <klargefile.h>
// Krusader includes
#include "normal_vfs.h"
#include "../Dialogs/krdialogs.h"
#include "../MountMan/kmountman.h"
#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"

normal_vfs::normal_vfs(QString url,QWidget* panel):vfs(panel){
  vfs_filesP = &vfs_files;
  vfs_files.setAutoDelete(true);
  
  vfs_origin = KURL::fromPathOrURL(url);
  vfs_type=NORMAL;
	
	// connect the watcher to vfs_slotDirty
  connect(&watcher,SIGNAL(dirty(const QString&)),this,SLOT(vfs_slotDirty()));
}

bool normal_vfs::vfs_refresh(const KURL& origin){
  QString path = origin.path(-1);

	// check that the new origin exists
	if ( !QDir(path).exists() ) return false;
    
  krConfig->setGroup("Advanced");
  if (krConfig->readBoolEntry("AutoMount",_AutoMount)) krMtMan.autoMount(path);
   	
	watcher.stopScan(); //stop watching the old dir
  if(  !origin.equals(vfs_getOrigin()) ){
		// and remove it from the list
		if( !vfs_getOrigin().isEmpty() ) watcher.removeDir(vfs_getOrigin().path(-1)); 
		watcher.addDir(origin.path(-1)); //start watching the new dir
	}

	// set the writable attribute to true, if that's not the case - the KIO job
  // will give the warnings and errors
	isWritable = true;
	
 	krConfig->setGroup("Look&Feel");
	bool hidden = krConfig->readBoolEntry("Show Hidden",_ShowHidden);
	krConfig->setGroup("General");
	bool mtm    = krConfig->readBoolEntry("Mimetype Magic",_MimetypeMagic);

	// set the origin...
  vfs_origin = origin;
	vfs_origin.cleanPath();
  // clear the the list
	vfs_files.clear();

	
	DIR* dir = opendir(path.local8Bit());
  if(!dir) return false;

  //change directory to the new directory
  chdir(path.local8Bit());

	if (!quietMode) emit startUpdate();

	struct dirent* dirEnt;
  QString name;
	KURL mimeUrl;
  char symDest[256];
  KDE_struct_stat stat_p;

	while( (dirEnt=readdir(dir)) != NULL ){
    name = QString::fromLocal8Bit(dirEnt->d_name);

		// show hidden files ?
		if ( !hidden && name.left(1) == "." ) continue ;
		// we dont need the ".",".." enteries
		if (name=="." || name == "..") continue;
	  	
	  KDE_lstat(vfs_workingDir().local8Bit()+"/"+name.local8Bit(),&stat_p);
	  KIO::filesize_t size = stat_p.st_size;
    QString perm = KRpermHandler::mode2QString(stat_p.st_mode);
	  bool symLink= S_ISLNK(stat_p.st_mode);
	  if( S_ISDIR(stat_p.st_mode) ) perm[0] = 'd';
	
	  bzero(symDest,256);
		mimeUrl.setPath(vfs_workingDir()+"/"+name);
	  QString mime=KMimeType::findByURL( mimeUrl,stat_p.st_mode,true,!mtm)->name();
	
	  if( S_ISLNK(stat_p.st_mode) ){  // who the link is pointing to ?
	    int endOfName=0;
	    endOfName=readlink(vfs_workingDir().local8Bit()+"/"+name.local8Bit(),symDest,256);
	    if ( endOfName != -1 ){
	      if ( QDir(symDest).exists() || mime.contains("directory") ) perm[0] = 'd';
	      //QString symTemp = (symDest[0]=='/' ? symDest : vfs_workingDir()+"/"+symDest);
	      if ( !QDir(vfs_workingDir()).exists(symDest)  ) mime = "Broken Link !";
      }
      else kdWarning() << "Failed to read link: "<< vfs_workingDir()+"/"+name<<endl;
	  }
	  	
	  // create a new virtual file object
    vfile* temp=new vfile(name,size,perm,stat_p.st_mtime,symLink,stat_p.st_uid,
                          stat_p.st_gid,mime,symDest,stat_p.st_mode);
    addToList(temp);
  }
	// clean up
	closedir(dir);
	
	if (!quietMode) emit endUpdate();
  watcher.startScan();

  return true;
}

// copy a file to the vfs (physical)	
void normal_vfs::vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir){
  watcher.stopScan(); // we will refresh manually this time...	

	KURL dest;
	dest.setPath(vfs_workingDir()+"/"+dir);

  KIO::Job* job = new KIO::CopyJob(*fileUrls,dest,mode,false,true );
  connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()) );
  if(mode == KIO::CopyJob::Move) // notify the other panel
    connect(job,SIGNAL(result(KIO::Job*)),toNotify,SLOT(vfs_refresh(KIO::Job*)) );
}

// remove a file from the vfs (physical)
void normal_vfs::vfs_delFiles(QStringList *fileNames){
	KURL::List filesUrls;
	KURL url;
	QDir local( vfs_workingDir() );
  vfile* vf;

  watcher.stopScan(); // we will refresh manually this time...	

	// names -> urls
	for(uint i=0 ; i<fileNames->count(); ++i){
		QString filename = (*fileNames)[i];
		vf = vfs_search(filename);
		url.setPath( vfs_workingDir()+"/"+filename);
		filesUrls.append(url);
	}
	KIO::Job *job;
	
	// delete of move to trash ?
	krConfig->setGroup("General");
	if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ){
	  job = new KIO::CopyJob(filesUrls,KGlobalSettings::trashPath(),KIO::CopyJob::Move,false,true );
	  connect(job,SIGNAL(result(KIO::Job*)),krApp,SLOT(changeTrashIcon()));
	}
	else
	  job = new KIO::DeleteJob(filesUrls, false, true);
	
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
}

// return a path to the file
KURL normal_vfs::vfs_getFile(const QString& name){	
  QString url;
	if ( vfs_workingDir() == "/" ) url = "/"+name;
	else url = vfs_workingDir()+"/"+name;

	return KURL::fromPathOrURL(url);
}

KURL::List* normal_vfs::vfs_getFiles(QStringList* names){
  KURL::List* urls = new KURL::List();
  for(QStringList::Iterator name = names->begin(); name != names->end(); ++name){
    urls->append( vfs_getFile(*name) );
  }
  return urls;
}

void normal_vfs::vfs_mkdir(const QString& name){
	if (!QDir(vfs_workingDir()).mkdir(name))
	  if (!quietMode) KMessageBox::sorry(krApp,i18n("Can't create a directory check your permissions."));
  vfs::vfs_refresh();
}

void normal_vfs::vfs_rename(const QString& fileName,const QString& newName){
  KURL::List fileUrls;
  KURL url , dest;

  watcher.stopScan(); // we will refresh manually this time...	

  url.setPath( vfs_workingDir()+"/"+fileName );
  fileUrls.append(url);
	dest.setPath(vfs_workingDir()+"/"+newName);

  KIO::Job *job = new KIO::CopyJob(fileUrls,dest,KIO::CopyJob::Move,false,false );
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
}

void normal_vfs::vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool* stop){
  if (stop && *stop) return;
  if (!name.contains("/")) name = vfs_workingDir()+"/"+name;
  if (name == "/proc") return;

  QFileInfo qfi(name);
  if(qfi.isSymLink() || !qfi.isDir()){ // single files are easy : )
    ++(*totalFiles);
    (*totalSize) += qfi.size();
  }
  else{  // handle directories
		QDir dir(name);
    // avoid a nasty crash on un-readable dirs
		if ( !qfi.isReadable() || !dir.exists() ) return;
		++(*totalDirs);
    dir.setFilter(QDir::All|QDir::Hidden);
	  dir.setSorting(QDir::Name|QDir::DirsFirst);
	
	  // recurse on all the files in the directory
	  QFileInfoList* fileList = const_cast<QFileInfoList*>(dir.entryInfoList());
	  for (QFileInfo* qfiP = fileList->first(); qfiP != 0; qfiP = fileList->next()){
      if (stop && *stop) return;
      if (qfiP->fileName() != "." && qfiP->fileName() != "..")
	      vfs_calcSpace(name+"/"+qfiP->fileName(),totalSize,totalFiles,totalDirs,stop);
	  }
  }
}

#include "normal_vfs.moc"
