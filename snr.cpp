#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <new>
#include <vector>

namespace EA {
	namespace Allocator {
		class ICoreAllocator
		{
		public:
			struct DebugParams
			{
				const char* const mName;
				const char* const mFileName;
				const int mLineNum;
			};
			virtual ~ICoreAllocator() {}
			virtual void* Alloc(unsigned int size, const char* name, unsigned int flags, unsigned int align, unsigned int alignOffset)
			{
				return _aligned_malloc(size, align);
			}
			virtual void* Alloc(unsigned int size, const char* name, unsigned int flags)
			{
				return _aligned_malloc(size, 4);
			}
			virtual void Free(void* block, unsigned int size)
			{
				return _aligned_free(block);
			}
			virtual void* AllocDebug(unsigned int size, DebugParams debugParams, unsigned int flags, unsigned int align, unsigned int alignOffset)
			{
				return Alloc(size, debugParams.mName, flags, align, alignOffset);
			}
			virtual void* AllocDebug(unsigned int size, DebugParams debugParams, unsigned int flags)
			{
				return Alloc(size, debugParams.mName, flags);
			}
		};
	}
}

namespace rw {
	namespace audio {
		namespace core {
			class DecoderRegistry;
			class EncoderRegistry;
			class StackAllocator;
			struct ListDNode
			{
			private:
				ListDNode* pnext;
				ListDNode* pprev;
			public:
				void SetNext(ListDNode* pnode)
				{
					pnext = pnode;
				}
				void SetPrev(ListDNode* pnode)
				{
					pprev = pnode;
				}
				ListDNode* GetNext()
				{
					return pnext;
				}
				ListDNode* GetPrev()
				{
					return pprev;
				}
			};
			struct ListDStack
			{
			private:
				ListDNode* phead;
			public:
				ListDStack() : phead(nullptr)
				{
				}
				void Reset()
				{
					phead = nullptr;
				}
				bool IsEmpty()
				{
					return phead == nullptr;
				}
				ListDNode* GetHead()
				{
					return phead;
				}
				void Push(ListDNode* pnode)
				{
					pnode->SetNext(phead);
					pnode->SetPrev(nullptr);
					if (phead)
					{
						phead->SetPrev(pnode);
					}
					phead = pnode;
				}
				ListDNode* Pop()
				{
					ListDNode* node = phead;
					if (node)
					{
						phead = node->GetNext();
						if (node->GetNext() != nullptr)
						{
							node->GetNext()->SetPrev(nullptr);
						}
					}
					return node;
				}
				void Remove(ListDNode* pnode)
				{
					if (pnode == phead)
					{
						phead = phead->GetNext();
					}
					if (pnode->GetPrev())
					{
						pnode->GetPrev()->SetNext(pnode->GetNext());
					}
					if (pnode->GetNext())
					{
						pnode->GetNext()->SetPrev(pnode->GetPrev());
					}
				}
			};
			class PlugIn;
			class PlugInRegistry;
			class System {
			private:
				static System* spInstance;
				StackAllocator* mpStackAllocator;
				void(*mpAssertImplementation)(System*, const char*, const char*, const char*, int);
				double mSystemTime;
				ListDStack mExpelledVoiceList;
				EA::Allocator::ICoreAllocator* mpAllocator;
				void* (*mpPhysicalAlloc)(unsigned int, unsigned int, unsigned int, const char*);
				void(*mpPhysicalFree)(void*);
				char* mpCommandBuffer;
				PlugIn* mpMasteringSubMix;
				PlugInRegistry* mpPlugInRegistry;
				DecoderRegistry* mpDecoderRegistry;
				EncoderRegistry* mpEncoderRegistry;
			public:
				System();
				static System* GetInstance() { return spInstance; }
				static void CreateInstance(EA::Allocator::ICoreAllocator* pAllocator);
				static void Assert(System*, const char*, const char*, const char*, int);
				void* Alloc(unsigned int size, char const* name, unsigned int alignment, EA::Allocator::ICoreAllocator* pAllocator);
				void Free(void* pMem, EA::Allocator::ICoreAllocator* pAllocator);
				void Lock();
				void Unlock();
				DecoderRegistry* GetDecoderRegistry();
				EncoderRegistry* GetEncoderRegistry();
				void Release();
			};
			class SampleBuffer
			{
			public:
				static unsigned int GetSize(unsigned int numChannels, unsigned int __formal, unsigned int* pAlignment, rw::audio::core::System* pSystem)
				{
					*pAlignment = 16;
					return 20;
				}
				static SampleBuffer* CreateInstance(unsigned int numChannels, unsigned int maxLocks, unsigned int maxSamples, void* pMem, void* pStorage, rw::audio::core::System* pSystem)
				{
					SampleBuffer* buf = (SampleBuffer*)pMem;
					buf->mNumChannels = numChannels;
					buf->mpSystem = pSystem;
					buf->mpStorage = (float*)pStorage;
					buf->mNumSamples = 0;
					buf->mMaxSamples = maxSamples;
					return buf;
				}
				static unsigned int CalculateStorageSize(unsigned int numChannels, unsigned int maxSamples)
				{
					return 4 * numChannels * maxSamples;
				}
				float* LockChannel(unsigned int channel)
				{
					return &mpStorage[mMaxSamples * channel];
				}
				void UnlockChannel(unsigned int channel)
				{
				}
				void SetNumSamples(unsigned int numSamples)
				{
					mNumSamples = numSamples;
				}
				unsigned int GetNumSamples()
				{
					return mNumSamples;
				}
			private:
				System* mpSystem;
				float* mpStorage;
				float* mpTempStore;
				unsigned short mNumSamples;
				unsigned short mMaxSamples;
				char mNumChannels;
			};
			class Decoder
			{
			public:
				struct RequestDesc
				{
					void* pSrc;
					void* pSeekData;
					int decoderSkip;
					int numSamples;
					int numBytes;
					unsigned char feedType;
					unsigned char seekDataVersion;
				};
				enum FeedType
				{
					FEEDTYPE_NEW = 0x0,
					FEEDTYPE_CONTINUE = 0x1,
				};
				virtual void FeedEvent(unsigned char) {}
				virtual ~Decoder() {}
				unsigned char Feed(void*, int, FeedType, int, void*, int, int);
				int Decode(SampleBuffer*, int);
				void Release();
				int GetSamplesRemaining(unsigned char);
				unsigned int GetInstanceSize() { return mInstanceSize; }
				System* GetSystem() { return mpSystemUseGetSystemAccessor; }
				unsigned int GetChannels() { return mNumChannels; }
				Decoder() {}
				void SetSystem(System* system) { mpSystemUseGetSystemAccessor = system; }
				RequestDesc* GetRequestDescArray() { return (RequestDesc*)((char*)this + mRequestDescOffset); }
				RequestDesc* GetRequestDesc(unsigned int slot) { return (RequestDesc*)((char*)this + sizeof(RequestDesc) * slot + mRequestDescOffset); }
				void SetChannels(unsigned int numChannels) { mNumChannels = numChannels; }
				SampleBuffer* GetSampleBuffer() { return (SampleBuffer*)((char*)this + mSampleBufferOffset); }
				RequestDesc* AdvanceSlot(unsigned char* pSlot); //todo
				void AdvanceDecodeState(int numSamples); //todo
				RequestDesc* GetCurrentRequestDesc(); //todo
				RequestDesc* GetDecodingRequestDesc()
				{
					RequestDesc* desc = (RequestDesc*)((char*)this + sizeof(RequestDesc) * mDecodeSlot + mRequestDescOffset);
					if (!desc->numSamples)
					{
						desc = 0;
					}
					return desc;
				}
				void ResetPrepareSlot()
				{
					mPrepareSlot = mDecodeSlot + 1;
					if (mPrepareSlot >= mMaxSlots)
					{
						mPrepareSlot = 0;
					}
				}
			private:
				System* mpSystemUseGetSystemAccessor;
				void* mpDecoder;
				void(*mpReleaseEvent)(Decoder*);
				void* mSampleBufferStorage;
				int(*mpDecodeEvent)(Decoder*, SampleBuffer*, int);
				unsigned int mGuid;
				int mDecodeSlotSamplesDecoded;
				unsigned int mInstanceSize;
				unsigned int mRequestDescOffset;
				unsigned int mSampleBufferOffset;
				unsigned short mDecodedSamplesAvailable;
				unsigned char mNumChannels;
				unsigned char mFeedSlot;
				unsigned char mPrepareSlot;
				unsigned char mDecodeSlot;
				unsigned char mMaxSlots;
				unsigned char mIsBlockBased;
				friend class DecoderRegistry;
			};
			class EALayer3Core;
			struct DecoderDesc
			{
				unsigned int (*pGetSize)(unsigned int, unsigned int*);
				bool (*pCreateInstanceEvent)(Decoder*);
				void (*pReleaseEvent)(Decoder*);
				int (*pDecodeEvent)(Decoder*, SampleBuffer*, int);
				unsigned int guid;
				unsigned short maxBlockSize;
			};
			class DecoderExtended
			{
			public:
				Decoder* mpDecoder;
				SampleBuffer* mpSampleBuffer;
				void* mpSampleBufferStorage;
				void Release();
				unsigned char Feed(void*, int, Decoder::FeedType);
				int Decode(float*, int);
				int Decode(short** const, int);
				int Decode(short* pDst, int samples); //todo
				int Decode(float** pDst, int samples); //todo
				void SetDecoder(Decoder* decoder)
				{
					mpDecoder = decoder;
				}
			private:
				static void TranslateF32ToS16(void*, void*, int);
				static DecoderExtended* CreateInstance(System*, unsigned int);
				friend class DecoderRegistry;
			};
			class ListNode
			{
			private:
				ListNode* pnext;
			public:
				void SetNext(ListNode* pnode)
				{
					pnext = pnode;
				}
				ListNode* GetNext()
				{
					return pnext;
				}
			};
			class ListQueue
			{
			private:
				ListNode* phead;
				ListNode* ptail;
				int entries;
			public:
				ListQueue() : phead(nullptr), ptail(nullptr), entries(0)
				{
				}
				void Reset()
				{
					phead = nullptr;
					ptail = nullptr;
					entries = 0;
				}
				ListNode* GetHead()
				{
					return phead;
				}
				void Push(ListNode* pnode)
				{
					pnode->SetNext(phead);
					if (!ptail)
					{
						ptail = pnode;
					}
					phead = pnode;
					entries++;
				}
				void PushTail(ListNode* pnode)
				{
					pnode->SetNext(0);
					if (phead)
					{
						ptail->SetNext(pnode);
					}
					else
					{
						phead = pnode;
					}
					ptail = pnode;
					entries++;
				}
				ListNode* Pop()
				{
					ListNode* node = phead;
					if (node)
					{
						phead = node->GetNext();
						if (!node->GetNext())
						{
							ptail = nullptr;
						}
						entries--;
					}
					return node;
				}
			};
			class DecoderRegistry
			{
			private:
				std::vector<DecoderDesc*> mDecoderDescList;
				System* mpSystem;
			public:
				static DecoderRegistry* CreateInstance(class System* pSystem);
				void* GetDecoderHandle(unsigned int guid);
				void RegisterDecoder(DecoderDesc* pDecoderDesc);
				DecoderExtended* DecoderExtendedFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem);
				Decoder* DecoderFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem);
				void RegisterAllDecoders();
				DecoderRegistry();
				void Release();
			};
			class EaLayer3DecBase : public Decoder {
			protected:
				static void ReleaseEvent(Decoder*);
				static unsigned int GetSize(unsigned int, unsigned int*);
				static bool CreateInstance(class rw::audio::core::Decoder*, char);
			private:
				void DecodePcm(float**, void*, unsigned int, unsigned int);
				int DecodeSpecialBlock(unsigned char*, float**, EALayer3Core*);
				void SkipBlocks();
				int DecodeGranule(unsigned char*, float**, EALayer3Core*, int*, int*, int*, int);
			public:
				static int DecodeEvent(Decoder*, SampleBuffer*, int);
			private:
				EALayer3Core* mpLoadedEALayer3Core;
				unsigned char* mpEncodedSample;
				unsigned char* mpEncodedDataEnd;
				rw::audio::core::EALayer3Core** mppEaLayer3Core;
				int mRemainingSamples;
				int mTotalChannels;
				int mNumEaLayer3CoreInstances;
				int mNextBlockSize;
				int mLatency;
				int mSkipSamples;
				char mVersion;
				bool mNewFeed;
			};
			class EaLayer31Dec : public EaLayer3DecBase {
			private:
				static bool CreateInstanceEvent(Decoder*);
				static DecoderDesc sDecoderDesc;
			public:
				static DecoderDesc* GetDecoderDesc();
			};
			DecoderDesc EaLayer31Dec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'EL31', 576 };
			class EaLayer32PcmDec : public EaLayer3DecBase {
			private:
				static bool CreateInstanceEvent(Decoder*);
				static DecoderDesc sDecoderDesc;
			public:
				static DecoderDesc* GetDecoderDesc();
			};
			DecoderDesc EaLayer32PcmDec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'L32P', 576 };
			class EaLayer32SpikeDec : public EaLayer3DecBase {
			private:
				static bool CreateInstanceEvent(Decoder*);
				static DecoderDesc sDecoderDesc;
			public:
				static DecoderDesc* GetDecoderDesc();
			};
			DecoderDesc EaLayer32SpikeDec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'L32S', 576 };
			class Pcm16BigDec : public Decoder {
			private:
				static unsigned int GetSize(unsigned int, unsigned int*);
				static bool CreateInstanceEvent(Decoder*);
				static DecoderDesc sDecoderDesc;
			public:
				static int DecodeEvent(Decoder*, SampleBuffer*, int);
				static DecoderDesc* GetDecoderDesc();
			private:
				unsigned int mpSrc;
				int mRemainingSamples;
			};
			DecoderDesc Pcm16BigDec::sDecoderDesc = { GetSize, CreateInstanceEvent, nullptr, DecodeEvent, 'P6B0', 0 };
			class Xas1Dec {
			private:
				static const float sShiftMulLut[13];
				static const float sFilterPairs[4][2];
				static unsigned int GetSize(unsigned int, unsigned int*);
				static bool CreateInstanceEvent(Decoder*);
				static DecoderDesc sDecoderDesc;
				void DecodeChannel(unsigned char*, float*);
			public:
				static int DecodeEvent(Decoder*, SampleBuffer*, int);
				static DecoderDesc* GetDecoderDesc();
			};
			DecoderDesc Xas1Dec::sDecoderDesc = { GetSize, CreateInstanceEvent, 0, DecodeEvent, 'Xas1', 128 };
#if 0
			class Encoder;
			struct EncoderDesc
			{
				unsigned int guid;
				Encoder* (*CreateInstance)(int, int, System*);
				void* listNode;
				unsigned short maxLatency;
				unsigned char seekDataVersion;
				unsigned char isSeekable;
				unsigned char isLoopInfoRequired;
			};
			class Encoder
			{
			public:
				virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) = 0;
				virtual int Flush(unsigned char*, int*, void*, int*, int) = 0;
				virtual int GetDataRateOverhead();
				virtual int GetSeekMemoryRequired();
				virtual void Release();
			protected:
				virtual ~Encoder();
			private:
				System* mpSystem;
				EncoderDesc* mpEncoderDesc;
				float mAverageDataRate;
				float mVbrQuality;
				int mCbrRate;
				int mLoopStart;
				int mLoopEnd;
				int mSampleRate;
				unsigned char mNumChannels;
				char mBitRateManagement;
				bool mIsChunked;
			};
			struct Xas1Enc : public Encoder
			{
			public:
				static EncoderDesc* GetEncoderDesc();
				virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) override;
				virtual int Flush(unsigned char*, int*, void*, int*, int) override;
			private:
				static Encoder* CreateInstance(int, int, System*);
				void CustomInterleaveXAS(unsigned char*, unsigned char*);
				void EncodeBlock(float*, unsigned char*, int);
				static EncoderDesc sEncoderDesc;
				static const float sFilterPairs[4][2];
				int mResidualSamples;
				float mResidueBuffer[8192];
			};
			EncoderDesc Xas1Enc::sEncoderDesc = { 'Xas1', CreateInstance, nullptr, 0, 0, 1, 0 };
			struct Pcm16BigEnc : public Encoder
			{
			public:
				static EncoderDesc* GetEncoderDesc();
				virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) override;
				virtual int Flush(unsigned char*, int*, void*, int*, int) override;
			private:
				static Encoder* CreateInstance(int, int, System*);
				static EncoderDesc sEncoderDesc;
			};
			EncoderDesc Pcm16BigEnc::sEncoderDesc = { 'P6B0', CreateInstance, nullptr, 0, 0, 1, 0 };
			struct EaLayer32Enc : public Encoder
			{
			public:
				virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) override;
				virtual int Flush(unsigned char*, int*, void*, int*, int) override;
				virtual int GetSeekMemoryRequired() override;
				virtual void Release() override;
			};
			struct EaLayer32PcmEnc : public EaLayer32Enc
			{
			public:
				static EncoderDesc* GetEncoderDesc();
			private:
				static Encoder* CreateInstance(int, int, System*);
				static EncoderDesc sEncoderDesc;
			};
			EncoderDesc EaLayer32PcmEnc::sEncoderDesc = { 'L32P', CreateInstance, nullptr, 1728, 0, 1, 0 };
			struct EaLayer32SpikeEnc : public Encoder
			{
			public:
				static EncoderDesc* GetEncoderDesc();
			private:
				static Encoder* CreateInstance(int, int, System*);
				static EncoderDesc sEncoderDesc;
			};
			EncoderDesc EaLayer32SpikeEnc::sEncoderDesc = { 'L32S', CreateInstance, nullptr, 1728, 0, 1, 0 };
			struct EaLayer3Enc : public Encoder
			{
			public:
				static EncoderDesc* GetEncoderDesc();
			private:
				static Encoder* CreateInstance(int, int, System*);
				static EncoderDesc sEncoderDesc;
			};
			EncoderDesc EaLayer3Enc::sEncoderDesc = { 'EL31', CreateInstance, nullptr, 1728, 0, 1, 0 };
#endif
			System::System()
			{
				mpStackAllocator = nullptr;
				mpAssertImplementation = Assert;
			}
			void System::CreateInstance(EA::Allocator::ICoreAllocator* pAllocator)
			{
				void* v = pAllocator->Alloc(sizeof(System), "rw::audio::core::System", 1, 16, 0);
				System* s = new(v) System;
				s->mpCommandBuffer = nullptr;
				spInstance = s;
				s->mpAllocator = pAllocator;
				s->mpPhysicalAlloc = nullptr;
				s->mpPhysicalFree = nullptr;
				s->mSystemTime = 0.0;
				s->mpMasteringSubMix = nullptr;
				s->mpPlugInRegistry = nullptr;
			}
			void System::Assert(System*, const char*, const char*, const char*, int)
			{
			}
			void* System::Alloc(unsigned int size, char const* name, unsigned int alignment, EA::Allocator::ICoreAllocator* pAllocator)
			{
				EA::Allocator::ICoreAllocator* a = pAllocator;
				if (!pAllocator)
				{
					a = mpAllocator;
				}
				return a->Alloc(size, name, 1, alignment, 0);
			}
			void System::Free(void* pMem, EA::Allocator::ICoreAllocator* pAllocator)
			{
				EA::Allocator::ICoreAllocator* a = pAllocator;
				if (!pAllocator)
				{
					a = mpAllocator;
				}
				a->Free(pMem, 0);
			}
			void System::Lock()
			{
			}
			void System::Unlock()
			{
			}
			DecoderRegistry* System::GetDecoderRegistry()
			{
				if (!mpDecoderRegistry)
				{
					mpDecoderRegistry = DecoderRegistry::CreateInstance(spInstance);
				}
				return mpDecoderRegistry;
			}
			EncoderRegistry* System::GetEncoderRegistry()
			{
				//todo
				return nullptr;
			}
			void System::Release()
			{
				if (mpDecoderRegistry)
				{
					mpDecoderRegistry->Release();
				}
				if (mpEncoderRegistry)
				{
					//mpEncoderRegistry->Release(); //todo
				}
				mpAllocator->Free(this, 0);
				spInstance = nullptr;
			}
			DecoderRegistry* DecoderRegistry::CreateInstance(class System* pSystem)
			{
				void *v = pSystem->Alloc(16, "rw::audio::core::DecoderRegistry", 16, 0);
				DecoderRegistry* d = new(v) DecoderRegistry();
				d->mpSystem = pSystem;
				return d;
			}
			void* DecoderRegistry::GetDecoderHandle(unsigned int guid)
			{
				for (DecoderDesc* d : mDecoderDescList)
				{
					if (d->guid == guid)
					{
						return d;
					}
				}
				return nullptr;
			}
			void DecoderRegistry::RegisterDecoder(DecoderDesc* pDecoderDesc)
			{
				mDecoderDescList.push_back(pDecoderDesc);
			}
			DecoderExtended* DecoderRegistry::DecoderExtendedFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem)
			{
				DecoderExtended* decoder = DecoderExtended::CreateInstance(pSystem, numChannels);
				decoder->SetDecoder(DecoderFactory(decoderHandle, numChannels, maxRequests, pSystem));
				return decoder;
			}
			Decoder* DecoderRegistry::DecoderFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem)
			{
				DecoderDesc* desc = (DecoderDesc*)decoderHandle;
				unsigned int alignment;
				unsigned int size = desc->pGetSize(numChannels, &alignment);
				unsigned int size2 = sizeof(Decoder::RequestDesc) * maxRequests + ((size + 7) & 0xFFFFFFF8);
				bool isblock = desc->maxBlockSize != 0;
				if (isblock)
				{
					size2 = ((size2 + 15) & 0xFFFFFFF0) + 20;
					if (alignment <= 16)
					{
						alignment = 16;
					}
				}
				if (!size2)
				{
					size2 = 52;
				}
				void* v = System::GetInstance()->Alloc(size2, "rw::audio::core::Decoder", alignment, nullptr);
				Decoder* d = new(v) Decoder;
				d->mpReleaseEvent = desc->pReleaseEvent;
				d->mSampleBufferStorage = nullptr;
				d->mNumChannels = numChannels;
				d->mpSystemUseGetSystemAccessor = pSystem;
				desc->pCreateInstanceEvent(d);
				d->mpDecoder = d;
				d->mpDecodeEvent = desc->pDecodeEvent;
				d->mMaxSlots = maxRequests;
				d->mInstanceSize = size2;
				d->mGuid = desc->guid;
				d->mDecodedSamplesAvailable = 0;
				d->mDecodeSlotSamplesDecoded = 0;
				d->mFeedSlot = 0;
				d->mPrepareSlot = 0;
				d->mDecodeSlot = 0;
				d->mIsBlockBased = isblock;
				unsigned int offset = ((unsigned int)&d->mpSystemUseGetSystemAccessor + size + 3) & 0xFFFFFFF8;
				d->mRequestDescOffset = offset - (unsigned int)d;
				if (isblock)
				{
					SampleBuffer *buffer = (SampleBuffer*)((offset + sizeof(Decoder::RequestDesc) * maxRequests + 15) & 0xFFFFFFF0);
					d->mSampleBufferOffset = (unsigned int)((unsigned int)buffer - (unsigned int)d);
					d->mSampleBufferStorage = pSystem->Alloc(4 * numChannels * desc->maxBlockSize, "Decoder block storage", 128, nullptr);
					SampleBuffer::CreateInstance(numChannels, 0, desc->maxBlockSize, buffer, d->mSampleBufferStorage, pSystem);
				}
				Decoder::RequestDesc* request = (Decoder::RequestDesc*)((char*)d + d->mRequestDescOffset);
				for (unsigned int i = 0; i < d->mMaxSlots; i++)
				{
					request->pSrc = 0;
					request->numSamples = 0;
					request++;
				}
				return d;
			}
			void DecoderRegistry::RegisterAllDecoders()
			{
				RegisterDecoder(EaLayer31Dec::GetDecoderDesc());
				RegisterDecoder(EaLayer32PcmDec::GetDecoderDesc());
				RegisterDecoder(EaLayer32SpikeDec::GetDecoderDesc());
				RegisterDecoder(Pcm16BigDec::GetDecoderDesc());
			}
			DecoderRegistry::DecoderRegistry()
			{
			}
			void DecoderRegistry::Release()
			{
				mpSystem->Free(this, nullptr);
			}
			class BitGetter
			{
			public:
				BitGetter() {}
				~BitGetter() {}
				void SetBitBuffer(void* pBuffer)
				{
					mpBitBuffer = (unsigned char*)pBuffer;
					mBitPosition = 0;
				}
				unsigned int GetBitPosition() { return mBitPosition; }
				unsigned int GetBits(unsigned int numBits)
				{
					unsigned int i1 = 0;
					if (!numBits)
					{
						return 0;
					}
					do
					{
						unsigned int i2 = mBitPosition & 7;
						unsigned int i3 = 8 - i2;
						if (8 - i2 > numBits)
						{
							i3 = numBits;
						}
						unsigned int i4 = mpBitBuffer[mBitPosition >> 3] >> (8 - i2 - i3);
						mBitPosition += i3;
						i1 = (i1 << i3) | ((1 << i3) - 1) & i4;
						numBits -= i3;
					} while (numBits);
					return i1;
				}
			private:
				unsigned char* mpBitBuffer;
				unsigned int mBitPosition;
			};
			enum SndPlayerCodec
			{
				SNDPLAYER_CODEC_XAS_INT = 0x0,
				SNDPLAYER_CODEC_EALAYER3_INT = 0x1,
				SNDPLAYER_CODEC_SIGN16BIG_INT = 0x2,
				SNDPLAYER_CODEC_EAXMA = 0x3,
				SNDPLAYER_CODEC_XAS1_INT = 0x4,
				SNDPLAYER_CODEC_EALAYER31_INT = 0x5,
				SNDPLAYER_CODEC_EALAYER32PCM_INT = 0x6,
				SNDPLAYER_CODEC_EALAYER32SPIKE_INT = 0x7,
				SNDPLAYER_CODEC_GCADPCM = 0x8,
				SNDPLAYER_CODEC_EASPEEX = 0x9,
				SNDPLAYER_CODEC_MAX = 0xA,
			};
			enum SndPlayerPlayType
			{
				SNDPLAYER_PLAYTYPE_RAM = 0x0,
				SNDPLAYER_PLAYTYPE_STREAM = 0x1,
				SNDPLAYER_PLAYTYPE_GIGASAMPLE = 0x2,
				SNDPLAYER_PLAYTYPE_MAX = 0x3,
			};
			struct FileInfo
			{
				int version;
				int codec;
				int channel_config;
				int sample_rate;
				int type;
				int loop_flag;
				int num_samples;
				int loop_start;
				int prefech_samples;
				int loop_offset;
			};
			class SNRReader
			{
			public:
				void GetFileInfo(FileInfo* info, unsigned char* buffer)
				{
					BitGetter b;
					b.SetBitBuffer(buffer);
					info->version = b.GetBits(4);
					info->codec = b.GetBits(4);
					info->channel_config = b.GetBits(6) + 1;
					info->sample_rate = b.GetBits(18);
					info->type = b.GetBits(2);
					info->loop_flag = b.GetBits(1);
					info->num_samples = b.GetBits(29);
					if (info->loop_flag)
					{
						info->loop_start = b.GetBits(32);
					}
					else
					{
						info->loop_start = -1;
					}
					if (info->type == SNDPLAYER_PLAYTYPE_GIGASAMPLE)
					{
						info->prefech_samples = b.GetBits(32);
					}
					if (info->loop_flag)
					{
						if (info->type == SNDPLAYER_PLAYTYPE_STREAM
							|| info->type == SNDPLAYER_PLAYTYPE_GIGASAMPLE && info->prefech_samples <= info->loop_start)
						{
							info->loop_offset = b.GetBits(32);
						}
						else
						{
							info->loop_offset = 0;
						}
					}
				}
				void ReadHeader(FileInfo* info)
				{
					unsigned char buffer[64];
					memset(buffer, 0, sizeof(buffer));
					size_t read = fread(buffer, 0, sizeof(buffer), f);
					bool b = false;
					unsigned char* buf2 = buffer;
					if (buffer[0] == 72)
					{
						b = true;
						buf2 = &buffer[4];
					}
					GetFileInfo(info, buf2);
					int i1 = 8;
				}
			private:
				FILE* f;
			};
			System* System::spInstance = nullptr;
		}
	}
}

bool mIsSse = true;

extern "C" void memset2(void* v, int i, size_t s)
{
	memset(v, i, s);
}

extern "C" void memcpy2(void* v, const void* v2, size_t s)
{
	memcpy(v, v2, s);
}
