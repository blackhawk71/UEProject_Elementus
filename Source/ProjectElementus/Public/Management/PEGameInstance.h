// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <EOSVoiceChatUser.h>
#include <Interfaces/OnlineSessionInterface.h>
#include <Engine/GameInstance.h>
#include "PEGameInstance.generated.h"

class FOnlineAccountCredentials;
struct FSessionDataHandler;
/**
 *
 */
USTRUCT(BlueprintType, Category = "Project Elementus | Structs")
struct FEOSVoiceChatChannelCredentials
{
	GENERATED_USTRUCT_BODY()

	FString ClientBaseUrl;
	FString ParticipantToken;
};

/* An exposed to BPs copy of EVoiceChatChannelType from VoiceChat.h */
UENUM(BlueprintType, Category = "Project Elementus | Enumerations")
enum class EEOSVoiceChatChannelType : uint8
{	
	/** Non positional/2d audio channel */
	NonPositional,
	/** Positional/3d audio channel */
	Positional,
	/** Echo channel. Will only ever have one player and will echo anything you say */
	Echo
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCreateSessionDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSessionsDelegate, const TArray<FSessionDataHandler>&, Sessions);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCancelFindSessionsDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJoinSessionDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDestroySessionDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUserLoginDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUserLogoutDelegate);

UCLASS(Category = "Project Elementus | Classes")
class UPEGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	explicit UPEGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void Init() override;

	virtual void OnMapLoadingStart(const FString& MapName);
	virtual void OnMapLoadingComplete(UWorld* InLoadedWorld);

public:	
	/* Initialize the voice chat framework */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void InitializeVoiceChatFramework();

	/* Shutdown the voice chat framework */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void ShutdownVoiceChatFramework();

	/* Connect the voice chat framework */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void ConnectVoiceChatFramework();

	/* Disconnect the voice chat framework */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void DisconnectVoiceChatFramework();

protected:
	FOnVoiceChatInitializeCompleteDelegate OnVoiceChatInitializeCompleteDelegate;
	void OnVoiceChatInitialized(const FVoiceChatResult& Result);

	FOnVoiceChatUninitializeCompleteDelegate OnVoiceChatUninitializeCompleteDelegate;
	void OnVoiceChatUninitialized(const FVoiceChatResult& Result);

	FOnVoiceChatConnectCompleteDelegate OnVoiceChatConnectCompleteDelegate;
	void OnVoiceChatConnected(const FVoiceChatResult& Result);

	FOnVoiceChatDisconnectCompleteDelegate OnVoiceChatDisconnectCompleteDelegate;
	void OnVoiceChatDisconnected(const FVoiceChatResult& Result);

public:
	/* Login to voice chat framework */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void LoginToVoiceChatFramework(const int32 LocalUserNum);

	/* Logout from voice chat framework */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void LogoutFromVoiceChatFramework(const int32 LocalUserNum);

protected:
	FOnVoiceChatLoginCompleteDelegate OnVoiceChatLoginCompleteDelegate;
	void OnVoiceChatLogin(const FString& PlayerName, const FVoiceChatResult& Result);

	FOnVoiceChatLogoutCompleteDelegate OnVoiceChatLogoutCompleteDelegate;
	void OnVoiceChatLogout(const FString& PlayerName, const FVoiceChatResult& Result);

public:
	/* Connect the user to a voice chat channel */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void ConnectVoiceChatToSessionChannel(const int32 LocalUserNum, const FString& ChannelName, const FEOSVoiceChatChannelCredentials Credentials, const EEOSVoiceChatChannelType ChannelType = EEOSVoiceChatChannelType::NonPositional);

	/* Disconnect the user from a voice chat channel */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void LeaveVoiceChatSessionChannel(const int32 LocalUserNum, const FString& ChannelName);

protected:
	FOnVoiceChatChannelJoinCompleteDelegate OnVoiceChatChannelJoinCompleteDelegate;
	void OnVoiceChatChannelJoined(const FString& ChannelName, const FVoiceChatResult& Result);

	FOnVoiceChatChannelLeaveCompleteDelegate OnVoiceChatChannelLeaveCompleteDelegate;
	void OnVoiceChatChannelLeft(const FString& ChannelName, const FVoiceChatResult& Result);

public:
	/* Create a new EOS Session */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Create EOS Session"))
	bool CreateEOSSession(const FSessionSettingsHandler SessionSettings);

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FCreateSessionDelegate CreateSessionDelegate;

	/* Find created EOS sessions */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Find EOS Sessions"))
	bool FindEOSSessions(const int32 LocalUserNum, const bool bIsLANQuery = false, const int32 MaxResults = 100);

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FFindSessionsDelegate FindSessionsDelegate;

	/* Cancel the current in progress Find EOS Sessions task */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Cancel Find EOS Sessions"))
	bool CancelFindEOSSessions();

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FCancelFindSessionsDelegate CancelFindSessionsDelegate;

	/* Join the specified EOS Session */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Join EOS Session"))
	bool JoinEOSSession(const int32 LocalUserNum, const FSessionDataHandler SessionData);

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FJoinSessionDelegate JoinSessionDelegate;

	/* Destroy the current EOS Session */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Destroy EOS Session"))
	bool DestroyEOSSession();

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FDestroySessionDelegate DestroySessionDelegate;

	/* Change the level from server-side with ?listen param */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void ServerTravelToLevel(const FName LevelName) const;

	/* Force client to travel to session level */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	void ClientTravelToSessionLevel(const int32 LocalUserNum) const;

	/* Get the data of existing sessions */
	UFUNCTION(BlueprintPure, Category = "Project Elementus | Functions")
	TArray<FSessionDataHandler> GetSessionsDataHandles() const;

	bool EOS_CreateSession(const uint8 HostingPlayerNum, const FOnlineSessionSettings& NewSessionSettings);
	bool EOS_FindSessions(const uint8 SearchingPlayerNum, const bool bIsLANQuery = false, const uint8 MaxResults = 100);
	bool EOS_JoinSession(const uint8 LocalUserNum, const FOnlineSessionSearchResult& DesiredSession);
	bool EOS_DestroySession();

protected:
	void OnSessionCreated(const FName SessionName, const bool bResult);
	void OnSessionsFound(const bool bResult);
	void OnCancelFindSessions(const bool bResult);
	void OnSessionJoined(const FName SessionName, const EOnJoinSessionCompleteResult::Type Result);
	void OnSessionDestroyed(const FName SessionName, const bool bResult);

	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	void OnSessionInviteAccepted(const bool bWasSuccessful, const int32 LocalUserNum, const FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

private:
	TSharedPtr<FOnlineSessionSearch> EOSSearchSettings;
	FString EOSCurrentSessionInfo;

public:
	/* Login to Epic Online Services */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Login to EOS"))
	bool EOSLogin(const int32 LocalUserNum, const FString& Token, const int32 Port, const bool bUsePortal = false);

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FUserLoginDelegate UserLoginDelegate;

	/* Logout from Epic Online Services */
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions", meta = (DisplayName = "Logout from EOS"))
	bool EOSLogout(const int32 LocalUserNum);

	UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
	FUserLogoutDelegate UserLogoutDelegate;

	bool EOS_Login(const uint8 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials);
	bool EOS_Logout(const uint8 LocalUserNum);

protected:
	void OnLoginComplete(const int32 LocalUserNum, const bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	void OnLogoutComplete(const int32 LocalUserNum, const bool bWasSuccessful);
};
