/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef _D_REQUEST_GROUP_H_
#define _D_REQUEST_GROUP_H_

#include "common.h"

#include <string>
#include <deque>

#include "SharedHandle.h"
#include "TransferStat.h"
#include "TimeA2.h"
#include "Request.h"
#include "DownloadResultCode.h"

namespace aria2 {

class DownloadEngine;
class SegmentMan;
class SegmentManFactory;
class Command;
class DownloadCommand;
class DownloadContext;
class PieceStorage;
class BtProgressInfoFile;
class Dependency;
class PreDownloadHandler;
class PostDownloadHandler;
class DiskWriterFactory;
class Option;
class Logger;
class RequestGroup;
class CheckIntegrityEntry;
class DownloadResult;
class URISelector;
class URIResult;
#ifdef ENABLE_BITTORRENT
class BtRuntime;
class PeerStorage;
#endif // ENABLE_BITTORRENT

class RequestGroup {
public:
  enum HaltReason {
    NONE,
    SHUTDOWN_SIGNAL,
    USER_REQUEST
  };
private:
  static int32_t _gidCounter;

  int32_t _gid;

  SharedHandle<Option> _option;

  size_t _numConcurrentCommand;

  /**
   * This is the number of connections used in streaming protocol(http/ftp)
   */
  unsigned int _numStreamConnection;

  unsigned int _numCommand;

  SharedHandle<SegmentMan> _segmentMan;
  SharedHandle<SegmentManFactory> _segmentManFactory;

  SharedHandle<DownloadContext> _downloadContext;

  SharedHandle<PieceStorage> _pieceStorage;

  bool _saveControlFile;

  SharedHandle<BtProgressInfoFile> _progressInfoFile;

  SharedHandle<DiskWriterFactory> _diskWriterFactory;

  SharedHandle<Dependency> _dependency;

  bool _fileAllocationEnabled;

  bool _preLocalFileCheckEnabled;

  bool _haltRequested;

  bool _forceHaltRequested;

  HaltReason _haltReason;

  std::deque<SharedHandle<PreDownloadHandler> > _preDownloadHandlers;

  std::deque<SharedHandle<PostDownloadHandler> > _postDownloadHandlers;

  std::deque<std::string> _acceptTypes;

  SharedHandle<URISelector> _uriSelector;

  Time _lastModifiedTime;

  unsigned int _fileNotFoundCount;

  // Timeout used for HTTP/FTP downloads.
  time_t _timeout;

#ifdef ENABLE_BITTORRENT
  WeakHandle<BtRuntime> _btRuntime;

  WeakHandle<PeerStorage> _peerStorage;
#endif // ENABLE_BITTORRENT

  // This flag just indicates that the downloaded file is not saved disk but
  // just sits in memory.
  bool _inMemoryDownload;

  unsigned int _maxDownloadSpeedLimit;

  unsigned int _maxUploadSpeedLimit;

  SharedHandle<URIResult> _lastUriResult;

  Logger* _logger;

  void validateFilename(const std::string& expectedFilename,
			const std::string& actualFilename) const;

  void initializePreDownloadHandler();

  void initializePostDownloadHandler();

  bool tryAutoFileRenaming();

  // Returns the result code of this RequestGroup.  If the download
  // finished, then returns downloadresultcode::FINISHED.  If the
  // download didn't finish and error result is available in
  // _uriResults, then last result code is returned.  Otherwise
  // returns downloadresultcode::UNKNOWN_ERROR.
  downloadresultcode::RESULT downloadResult() const;
public:
  RequestGroup(const SharedHandle<Option>& option);

  ~RequestGroup();
  /**
   * Reinitializes SegmentMan based on current property values and
   * returns new one.
   */
  const SharedHandle<SegmentMan>& initSegmentMan();

  const SharedHandle<SegmentMan>& getSegmentMan() const
  {
    return _segmentMan;
  }

  // Returns first bootstrap commands to initiate a download.
  // If this is HTTP/FTP download and file size is unknown, only 1 command
  // (usually, HttpInitiateConnection or FtpInitiateConnection) will be created.
  void createInitialCommand(std::deque<Command*>& commands,
			    DownloadEngine* e);

  void createNextCommandWithAdj(std::deque<Command*>& commands,
				DownloadEngine* e, int numAdj);

  void createNextCommand(std::deque<Command*>& commands,
			 DownloadEngine* e, unsigned int numCommand);
  
  bool downloadFinished() const;

  bool allDownloadFinished() const;

  void closeFile();

  std::string getFirstFilePath() const;

  uint64_t getTotalLength() const;

  uint64_t getCompletedLength() const;

  /**
   * Compares expected filename with specified actualFilename.
   * The expected filename refers to FileEntry::getBasename() of the first
   * element of DownloadContext::getFileEntries()
   */
  void validateFilename(const std::string& actualFilename) const;

  void validateTotalLength(uint64_t expectedTotalLength,
			   uint64_t actualTotalLength) const;

  void validateTotalLength(uint64_t actualTotalLength) const;

  void setSegmentManFactory(const SharedHandle<SegmentManFactory>& segmentManFactory);

  void setNumConcurrentCommand(unsigned int num)
  {
    _numConcurrentCommand = num;
  }

  unsigned int getNumConcurrentCommand() const
  {
    return _numConcurrentCommand;
  }

  int32_t getGID() const
  {
    return _gid;
  }

  TransferStat calculateStat();

  const SharedHandle<DownloadContext>& getDownloadContext() const
  {
    return _downloadContext;
  }

  void setDownloadContext(const SharedHandle<DownloadContext>& downloadContext)
  {
    _downloadContext = downloadContext;
  }

  const SharedHandle<PieceStorage>& getPieceStorage() const
  {
    return _pieceStorage;
  }

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  void setProgressInfoFile(const SharedHandle<BtProgressInfoFile>& progressInfoFile);

  void increaseStreamConnection();

  void decreaseStreamConnection();

  // Returns the number of connections used in HTTP(S)/FTP.
  unsigned int getNumStreamConnection() { return _numStreamConnection; }

  unsigned int getNumConnection() const;

  void increaseNumCommand();

  void decreaseNumCommand();

  unsigned int getNumCommand() const
  {
    return _numCommand;
  }

  // TODO is it better to move the following 2 methods to SingleFileDownloadContext?
  void setDiskWriterFactory(const SharedHandle<DiskWriterFactory>& diskWriterFactory);

  const SharedHandle<DiskWriterFactory>& getDiskWriterFactory() const
  {
    return _diskWriterFactory;
  }

  void setFileAllocationEnabled(bool f)
  {
    _fileAllocationEnabled = f;
  }

  bool isFileAllocationEnabled() const
  {
    return _fileAllocationEnabled;
  }

  bool needsFileAllocation() const;

  /**
   * Setting _preLocalFileCheckEnabled to false, then skip the check to see
   * if a file is already exists and control file exists etc.
   * Always open file with DiskAdaptor::initAndOpenFile()
   */
  void setPreLocalFileCheckEnabled(bool f)
  {
    _preLocalFileCheckEnabled = f;
  }

  bool isPreLocalFileCheckEnabled() const
  {
    return _preLocalFileCheckEnabled;
  }

  void setHaltRequested(bool f, HaltReason = SHUTDOWN_SIGNAL);

  void setForceHaltRequested(bool f, HaltReason = SHUTDOWN_SIGNAL);

  bool isHaltRequested() const
  {
    return _haltRequested;
  }

  bool isForceHaltRequested() const
  {
    return _forceHaltRequested;
  }

  void dependsOn(const SharedHandle<Dependency>& dep);

  bool isDependencyResolved();

  void releaseRuntimeResource(DownloadEngine* e);

  void postDownloadProcessing(std::deque<SharedHandle<RequestGroup> >& groups);

  void addPostDownloadHandler(const SharedHandle<PostDownloadHandler>& handler);

  void clearPostDowloadHandler();

  void preDownloadProcessing();

  void addPreDownloadHandler(const SharedHandle<PreDownloadHandler>& handler);

  void clearPreDowloadHandler();

  void processCheckIntegrityEntry(std::deque<Command*>& commands,
				  const SharedHandle<CheckIntegrityEntry>& entry,
				  DownloadEngine* e);

  void initPieceStorage();

  bool downloadFinishedByFileLength();

  void loadAndOpenFile(const SharedHandle<BtProgressInfoFile>& progressInfoFile);

  void shouldCancelDownloadForSafety();

  void adjustFilename(const SharedHandle<BtProgressInfoFile>& infoFile);

  SharedHandle<DownloadResult> createDownloadResult() const;

  const SharedHandle<Option>& getOption() const
  {
    return _option;
  }

  void reportDownloadFinished();

  const std::deque<std::string>& getAcceptTypes() const
  {
    return _acceptTypes;
  }

  void addAcceptType(const std::string& type);

  void removeAcceptType(const std::string& type);

  static const std::string ACCEPT_METALINK;

  void setURISelector(const SharedHandle<URISelector>& uriSelector);

  const SharedHandle<URISelector>& getURISelector() const
  {
    return _uriSelector;
  }

  void applyLastModifiedTimeToLocalFiles();

  void updateLastModifiedTime(const Time& time);

  void increaseAndValidateFileNotFoundCount();

  // Just set inMemoryDownload flag true.
  void markInMemoryDownload();

  // Returns inMemoryDownload flag.
  bool inMemoryDownload() const
  {
    return _inMemoryDownload;
  }

  void setTimeout(time_t timeout);

  time_t getTimeout() const
  {
    return _timeout;
  }

  // Returns true if current download speed exceeds
  // _maxDownloadSpeedLimit.  Always returns false if
  // _maxDownloadSpeedLimit == 0.  Otherwise returns false.
  bool doesDownloadSpeedExceed();

  // Returns true if current upload speed exceeds
  // _maxUploadSpeedLimit. Always returns false if
  // _maxUploadSpeedLimit == 0. Otherwise returns false.
  bool doesUploadSpeedExceed();

  unsigned int getMaxDownloadSpeedLimit() const
  {
    return _maxDownloadSpeedLimit;
  }

  void setMaxDownloadSpeedLimit(unsigned int speed)
  {
    _maxDownloadSpeedLimit = speed;
  }

  unsigned int getMaxUploadSpeedLimit() const
  {
    return _maxUploadSpeedLimit;
  }

  void setMaxUploadSpeedLimit(unsigned int speed)
  {
    _maxUploadSpeedLimit = speed;
  }

  void setLastUriResult(std::string uri, downloadresultcode::RESULT result);

  void saveControlFile() const;

  void removeControlFile() const;

  void enableSaveControlFile() { _saveControlFile = true; }

  void disableSaveControlFile() { _saveControlFile = false; }

  static void resetGIDCounter() { _gidCounter = 0; }
};

typedef SharedHandle<RequestGroup> RequestGroupHandle;
typedef WeakHandle<RequestGroup> RequestGroupWeakHandle;
typedef std::deque<RequestGroupHandle> RequestGroups;

} // namespace aria2

#endif // _D_REQUEST_GROUP_H_
