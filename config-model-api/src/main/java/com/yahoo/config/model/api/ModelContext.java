// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.model.api;

import com.yahoo.component.Version;
import com.yahoo.config.application.api.ApplicationPackage;
import com.yahoo.config.application.api.DeployLogger;
import com.yahoo.config.application.api.FileRegistry;
import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.AthenzDomain;
import com.yahoo.config.provision.DockerImage;
import com.yahoo.config.provision.HostName;
import com.yahoo.config.provision.Zone;

import java.io.File;
import java.net.URI;
import java.time.Duration;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * Model context containing state provided to model factories.
 *
 * @author Ulf Lilleengen
 */
public interface ModelContext {

    ApplicationPackage applicationPackage();
    Optional<Model> previousModel();
    Optional<ApplicationPackage> permanentApplicationPackage();
    Optional<HostProvisioner> hostProvisioner();
    Provisioned provisioned();
    DeployLogger deployLogger();
    ConfigDefinitionRepo configDefinitionRepo();
    FileRegistry getFileRegistry();
    Properties properties();
    default Optional<File> appDir() { return Optional.empty();}

    /** The Docker image repo we want to use for images for this deployment (optional, will use default if empty) */
    default Optional<DockerImage> wantedDockerImageRepo() { return Optional.empty(); }

    /** The Vespa version this model is built for */
    Version modelVespaVersion();

    /** The Vespa version we want nodes to become */
    Version wantedNodeVespaVersion();

    /** Warning: As elsewhere in this package, do not make backwards incompatible changes that will break old config models! */
    interface Properties {
        boolean multitenant();
        ApplicationId applicationId();
        List<ConfigServerSpec> configServerSpecs();
        HostName loadBalancerName();
        URI ztsUrl();
        String athenzDnsSuffix();
        boolean hostedVespa();
        Zone zone();
        Set<ContainerEndpoint> endpoints();
        boolean isBootstrap();
        boolean isFirstTimeDeployment();

        // TODO: Only needed for LbServicesProducerTest
        default boolean useDedicatedNodeForLogserver() { return true; }

        default Optional<EndpointCertificateSecrets> endpointCertificateSecrets() { return Optional.empty(); }

        // TODO Revisit in May or June 2020
        double defaultTermwiseLimit();

        default int defaultNumResponseThreads() { return 2; }

        // TODO(bjorncs) Temporary feature flag
        double threadPoolSizeFactor();

        // TODO(bjorncs) Temporary feature flag
        double queueSizeFactor();

        /// Default setting for the gc-options attribute if not specified explicit by application
        String jvmGCOptions();

        // Select sequencer type use while feeding.
        String feedSequencerType();
        String responseSequencerType();
        boolean skipCommunicationManagerThread();
        boolean skipMbusRequestThread();
        boolean skipMbusReplyThread();
        boolean tlsUseFSync();
        String tlsCompressionType();
        double visibilityDelay();

        boolean useContentNodeBtreeDb();

        boolean useThreePhaseUpdates();

        // TODO Remove on 7.XXX when this is default on.
        boolean useDirectStorageApiRpc();

        // TODO Remove on 7.XXX when this is default on.
        boolean useFastValueTensorImplementation();

        // TODO(bjorncs) Temporary feature flag
        default String proxyProtocol() { return "https+proxy-protocol"; }

        default Optional<AthenzDomain> athenzDomain() { return Optional.empty(); }

        // TODO(mpolden): Remove after May 2020
        default boolean useDedicatedNodesWhenUnspecified() { return true; }

        Optional<ApplicationRoles> applicationRoles();

        // TODO(bjorncs): Temporary feature flag, revisit August 2020
        default Duration jdiscHealthCheckProxyClientTimeout() { return Duration.ofMillis(100); }

        // TODO(bjorncs): Temporary feature flag
        default double feedCoreThreadPoolSizeFactor() { return 4.0; }

        default Quota quota() {
            return Quota.unlimited();
        }

        // TODO(bjorncs): Temporary feature flag
        default boolean useNewRestapiHandler() { return false; }

        // TODO(mortent): Temporary feature flag
        default boolean useAccessControlTlsHandshakeClientAuth() { return false; }

        // TODO(bjorncs): Temporary feature flag
        default double jettyThreadpoolSizeFactor() { return 0.0; }

    }

}
