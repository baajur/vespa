// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * @class storage::FileStorManager
 * @ingroup filestorage
 *
 * @version $Id$
 */

#pragma once

#include "filestorhandler.h"
#include "filestormetrics.h"
#include <vespa/vespalib/util/document_runnable.h>
#include <vespa/vespalib/util/isequencedtaskexecutor.h>
#include <vespa/document/bucket/bucketid.h>
#include <vespa/persistence/spi/persistenceprovider.h>
#include <vespa/storage/bucketdb/storbucketdb.h>
#include <vespa/storage/common/messagesender.h>
#include <vespa/storage/common/servicelayercomponent.h>
#include <vespa/storage/common/statusmessages.h>
#include <vespa/storage/common/storagelinkqueued.h>
#include <vespa/config-stor-filestor.h>
#include <vespa/storage/persistence/diskthread.h>

#include <vespa/storage/persistence/provider_error_wrapper.h>
#include <vespa/storage/common/nodestateupdater.h>
#include <vespa/storageframework/generic/status/htmlstatusreporter.h>

#include <vespa/config/subscription/configuri.h>
#include <vespa/config/helper/ifetchercallback.h>
#include <vespa/config/config.h>

namespace storage {
namespace api {
    class ReturnCode;
    class StorageReply;
}

struct FileStorManagerTest;
class ReadBucketList;
class BucketOwnershipNotifier;
class AbortBucketOperationsCommand;
struct DoneInitializeHandler;
class PersistenceHandler;

class FileStorManager : public StorageLinkQueued,
                        public framework::HtmlStatusReporter,
                        public StateListener,
                        private config::IFetcherCallback<vespa::config::content::StorFilestorConfig>,
                        public MessageSender
{
    ServiceLayerComponentRegister   & _compReg;
    ServiceLayerComponent             _component;
    spi::PersistenceProvider        & _providerCore;
    ProviderErrorWrapper              _providerErrorWrapper;
    spi::PersistenceProvider        * _provider;
    DoneInitializeHandler&            _init_handler;
    const document::BucketIdFactory & _bucketIdFactory;

    std::vector<std::unique_ptr<ServiceLayerComponent>> _persistenceComponents;
    std::vector<std::unique_ptr<PersistenceHandler>> _persistenceHandlers;
    std::vector<std::unique_ptr<DiskThread>>         _threads;
    std::unique_ptr<BucketOwnershipNotifier> _bucketOwnershipNotifier;

    std::unique_ptr<vespa::config::content::StorFilestorConfig> _config;
    config::ConfigFetcher _configFetcher;
    uint32_t              _threadLockCheckInterval; // In seconds
    bool                  _failDiskOnError;
    std::shared_ptr<FileStorMetrics> _metrics;
    std::unique_ptr<FileStorHandler> _filestorHandler;
    std::unique_ptr<vespalib::ISequencedTaskExecutor> _sequencedExecutor;
    bool       _closed;
    std::mutex _lock;

    friend struct FileStorManagerTest;

public:
    FileStorManager(const config::ConfigUri &, spi::PersistenceProvider&,
                    ServiceLayerComponentRegister&, DoneInitializeHandler&);
    FileStorManager(const FileStorManager &) = delete;
    FileStorManager& operator=(const FileStorManager &) = delete;

    ~FileStorManager() override;

    void print(std::ostream& out, bool verbose, const std::string& indent) const override;

    FileStorHandler& getFileStorHandler() {
        return *_filestorHandler;
    };

    spi::PersistenceProvider& getPersistenceProvider() {
        return *_provider;
    }
    ProviderErrorWrapper& error_wrapper() noexcept {
        return _providerErrorWrapper;
    }

    void handleNewState() override;

    // Must be called exactly once at startup _before_ storage chain is opened.
    // This function expects that no external messages may arrive prior to, or
    // concurrently with this call, such as client operations or cluster controller
    // node state requests.
    // By ensuring that this function is called prior to chain opening, this invariant
    // shall be upheld since no RPC/MessageBus endpoints have been made available
    // yet at that point in time.
    void initialize_bucket_databases_from_provider();

private:
    void configure(std::unique_ptr<vespa::config::content::StorFilestorConfig> config) override;
    PersistenceHandler & createRegisteredHandler(const ServiceLayerComponent & component);
    VESPA_DLL_LOCAL PersistenceHandler & getThreadLocalHandler();

    void replyWithBucketNotFound(api::StorageMessage&, const document::Bucket&);

    void replyDroppedOperation(api::StorageMessage& msg,
                               const document::Bucket& bucket,
                               api::ReturnCode::Result returnCode,
                               vespalib::stringref reason);

    StorBucketDatabase::WrappedEntry ensureConsistentBucket(
            const document::Bucket& bucket,
            api::StorageMessage& msg,
            const char* callerId);

    bool validateApplyDiffCommandBucket(api::StorageMessage& msg, const StorBucketDatabase::WrappedEntry&);
    bool validateDiffReplyBucket(const StorBucketDatabase::WrappedEntry&, const document::Bucket&);

    StorBucketDatabase::WrappedEntry mapOperationToDisk(api::StorageMessage&, const document::Bucket&);
    StorBucketDatabase::WrappedEntry mapOperationToBucketAndDisk(api::BucketCommand&, const document::DocumentId*);
    bool handlePersistenceMessage(const std::shared_ptr<api::StorageMessage>&);

    // Document operations
    bool onPut(const std::shared_ptr<api::PutCommand>&) override;
    bool onUpdate(const std::shared_ptr<api::UpdateCommand>&) override;
    bool onGet(const std::shared_ptr<api::GetCommand>&) override;
    bool onRemove(const std::shared_ptr<api::RemoveCommand>&) override;
    bool onRevert(const std::shared_ptr<api::RevertCommand>&) override;
    bool onStatBucket(const std::shared_ptr<api::StatBucketCommand>&) override;

    // Bucket operations
    bool onRemoveLocation(const std::shared_ptr<api::RemoveLocationCommand>&) override;
    bool onCreateBucket(const std::shared_ptr<api::CreateBucketCommand>&) override;
    bool onDeleteBucket(const std::shared_ptr<api::DeleteBucketCommand>&) override;
    bool onMergeBucket(const std::shared_ptr<api::MergeBucketCommand>&) override;
    bool onGetBucketDiff(const std::shared_ptr<api::GetBucketDiffCommand>&) override;
    bool onGetBucketDiffReply(const std::shared_ptr<api::GetBucketDiffReply>&) override;
    bool onApplyBucketDiff(const std::shared_ptr<api::ApplyBucketDiffCommand>&) override;
    bool onApplyBucketDiffReply(const std::shared_ptr<api::ApplyBucketDiffReply>&) override;
    bool onJoinBuckets(const std::shared_ptr<api::JoinBucketsCommand>&) override;
    bool onSplitBucket(const std::shared_ptr<api::SplitBucketCommand>&) override;
    bool onSetBucketState(const std::shared_ptr<api::SetBucketStateCommand>&) override;
    bool onNotifyBucketChangeReply(const std::shared_ptr<api::NotifyBucketChangeReply>&) override { return true; }

    // Other
    bool onInternal(const std::shared_ptr<api::InternalCommand>&) override;
    bool onInternalReply(const std::shared_ptr<api::InternalReply>&) override;

    void handleAbortBucketOperations(const std::shared_ptr<AbortBucketOperationsCommand>&);
    void sendCommand(const std::shared_ptr<api::StorageCommand>&) override;
    void sendReply(const std::shared_ptr<api::StorageReply>&) override;
    void sendReplyDirectly(const std::shared_ptr<api::StorageReply>&) override;
    void sendUp(const std::shared_ptr<api::StorageMessage>&) override;
    void onClose() override;
    void onFlush(bool downwards) override;
    void reportHtmlStatus(std::ostream&, const framework::HttpUrlPath&) const override;
    void storageDistributionChanged() override;
    void updateState();
    void propagateClusterStates();
    void update_reported_state_after_db_init();
};

} // storage
