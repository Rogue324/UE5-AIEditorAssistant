#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"

class IAIEditorAssistantChatSessionStore
{
public:
    virtual ~IAIEditorAssistantChatSessionStore() = default;

    virtual bool LoadSessionIndex(FAIEditorAssistantChatSessionIndex& OutIndex) const = 0;
    virtual bool SaveSessionIndex(const FAIEditorAssistantChatSessionIndex& SessionIndex) const = 0;
    virtual bool LoadSession(const FString& SessionId, FAIEditorAssistantChatSession& OutSession) const = 0;
    virtual bool SaveSession(const FAIEditorAssistantChatSession& Session) const = 0;
    virtual void DeleteSession(const FString& SessionId) const = 0;
};

class FAIEditorAssistantFileChatSessionStore : public IAIEditorAssistantChatSessionStore
{
public:
    virtual bool LoadSessionIndex(FAIEditorAssistantChatSessionIndex& OutIndex) const override;
    virtual bool SaveSessionIndex(const FAIEditorAssistantChatSessionIndex& SessionIndex) const override;
    virtual bool LoadSession(const FString& SessionId, FAIEditorAssistantChatSession& OutSession) const override;
    virtual bool SaveSession(const FAIEditorAssistantChatSession& Session) const override;
    virtual void DeleteSession(const FString& SessionId) const override;

private:
    FString GetChatsDirectory() const;
    FString GetSessionIndexPath() const;
    FString GetSessionPath(const FString& SessionId) const;
    void EnsureChatsDirectoryExists() const;
};
