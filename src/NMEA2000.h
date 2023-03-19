/* 
 * NMEA2000.h
 *
 * The MIT License
 *
 * Copyright (c) 2015-2023 Timo Lappalainen, Kave Oy, www.kave.fi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

/*************************************************************************//**
 * \file  NMEA2000.h
 * \brief This file contains the class tNMEA2000, which is consists the main
 *        functionality of the library
 * 
 * With NMEA2000 device class you can send, read and forward messages to 
 * NMEA2000 bus. As default library creates a system, which acts like Actisense 
 * NGT NMEA2000->PC interface forwarding all messages from bus to PC. By 
 * hanging mode to N2km_NodeOnly, one can make e.g. temperature source
 * device to NMEA2000 bus.
 * 
 * For detailed description see \ref tNMEA2000.
 * 
 */

#ifndef _NMEA2000_H_
#define _NMEA2000_H_

#include "NMEA2000_CompilerDefns.h"
#include "N2kStream.h"
#include "N2kMsg.h"
#include "N2kCANMsg.h"
#include "N2kTimer.h"

#if defined(_MSC_VER)
#define __attribute__(x)
#endif

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)
#include "N2kGroupFunction.h"
#endif
/** \brief PGN for an Iso Address Claim message */
#define N2kPGNIsoAddressClaim 60928L
/** \brief PGN for a Production Information message */
#define N2kPGNProductInformation 126996L
/** \brief PGN for an Configuration Information message */
#define N2kPGNConfigurationInformation 126998L

// Document says for leghts 33,40,24,32, but then values
// has not been translated right on devices.
/** \brief Max length of ModelID
 *  Document says for leghts 33 but then values has not
 *  been translated right on devices. */
#define Max_N2kModelID_len 32
/** \brief Max length of Software Code
 *  Document says for leghts 40 but then values has not
 *  been translated right on devices. */
#define Max_N2kSwCode_len 32
/** \brief Max length of Model Version
 *  Document says for leghts 24 but then values has not
 *  been translated right on devices. */
#define Max_N2kModelVersion_len 32
/** \brief Max length of SerialCode
 *  Document says for leghts 32 but then values has not
 *  been translated right on devices. */
#define Max_N2kModelSerialCode_len 32

/** \brief Define length of longest info string
 * Define length of longest info string (from \ref 
 * Max_N2kModelID_len, \ref Max_N2kSwCode_len, \ref 
 * Max_N2kModelVersion_len, \ref Max_N2kModelSerialCode_len) 
 * + 1 termination char
*/
#define Max_N2kProductInfoStrLen 33

/************************************************************************//**
 * \brief Max length of Configuration Info Fields
 * 
 * I do not know what standard says about max field length, but according 
 * to tests NMEAReader crashed with lenght >=90. Some device was reported
 * not to work string length over 70.
 */
#define Max_N2kConfigurationInfoField_len 71  // 70 + '/0'

/** \brief Message buffer time*/
#define Max_N2kMsgBuf_Time 100
/** \brief Number of message groups */
#define N2kMessageGroups 2
/** \brief Max CAN Bus Address given by the library*/
#define N2kMaxCanBusAddress 251
/** \brief Null Address (???)*/
#define N2kNullCanBusAddress 254

/************************************************************************//**
 * \class tNMEA2000
 * \brief NMEA2000 device class definition.
 * \ingroup group_core
 * 
 * With NMEA2000 device class you can send, read and forward messages to 
 * NMEA2000 bus. As default library creates a system, which acts like Actisense 
 * NGT NMEA2000->PC interface forwarding all messages from bus to PC. By 
 * hanging mode to N2km_NodeOnly, one can make e.g. temperature source
 * device to NMEA2000 bus.
 * 
 * \note Each device on NMEA2000 bus should have own address on range 0-253. 
 * 
 * This class uses J1939 automatic address claiming (or dynamic addressing).
 * So that you can start your device with some address set by method
 * \ref tNMEA2000::SetMode(). It is also important to set your device
 * "name" with method \ref tNMEA2000::SetDeviceInformation() so that 
 * it would be unique.
 * 
 * If you do not set "name" to unique, you devices changes address on start 
 * randomly. In principle they should still work fine.
 * It is also good idea to save device address to the EEPROM. In this way if 
 * you connect two of your devices to the bus, they will do the automatic 
 * address claiming. If later save address to the EEPROM and use that on next 
 * start, they does not need to change address anymore. See also method 
 * \ref tNMEA2000::ReadResetAddressChanged().
 */
class tNMEA2000
{
public:
  /************************************************************************//**
   * \brief Check if the given PGN is proprietary
   *
   * PGNs that are proprietary by definition: 61184, 65280 - 65535, 126720,
   * 130816 - 131071
   * 
   * \param PGN PGN to be checked
   * \return true -> for PGNs:
   *    - 126720L
   *    - 130816L
   *    - 131071L
   *    - 65280L ... 65535L
   * \return false 
   */
  static bool IsProprietaryMessage(unsigned long PGN);
  /************************************************************************//**
   * \brief Clears a char array buffer with 0s
   *
   * \param MaxLen  Max length of the buffer
   * \param buf     Pointer to the buffer
   */
  static void ClearCharBuf(size_t MaxLen, char *buf);
  /************************************************************************//**
   * \brief Setting up a Char Buffer
   * 
   * This functions copies the a to the char buffer and terminates it with 0.
   *
   * \param str     String 
   * \param MaxLen  Max length of the buffer
   * \param buf     Pointer to the buffer
   */
  static void SetCharBuf(const char *str, size_t MaxLen, char *buf);  
  /************************************************************************//**
   * \brief Setting up a clean Char Buffer
   * 
   * This functions clears an existiong buffer using \ref ClearCharBuf, 
   * copies the a string to the char buffer and terminates it with 0.
   *
   * \param str     String 
   * \param MaxLen  Max length of the buffer
   * \param buf     Pointer to the buffer
   */
  static void ClearSetCharBuf(const char *str, size_t MaxLen, char *buf);

  /************************************************************************//**
   * \brief Delivers Max out of A an B
   *
   * As max and min are not available on all systems, so use own definition.
   * 
   * \tparam T   Datatype
   * \param a    Value A
   * \param b    Value B
   * \return T 
   */
  template <typename T> static T N2kMax(T a, T b) { return (a>b?a:b); }
  
  /************************************************************************//**
   * \brief Delivers Min out of A an B
   *
   * As max and min are not available on all systems, so use own definition.
   * 
   * \tparam T   Datatype
   * \param a    Value A
   * \param b    Value B
   * \return T 
   */
  template <typename T> static T N2kMin(T a, T b) { return (a<b?a:b); }

  /************************************************************************//**
   * \struct  tProductInformation
   * \brief   Structure that holds all the product information
   *
   * It is importend that every device has proper product information 
   * available. This struct holds all the data and provides several 
   * helper functions.
   * 
   */
  struct tProductInformation {

      /** \brief Version of NMEA2000 Standard that is supported */
      unsigned short N2kVersion;
      /** \brief Product Code of the device*/
      unsigned short ProductCode;
      // 
      /** \brief Max length of ModelID
       * Note that we reserve one extra char for null termination  */
      char N2kModelID[Max_N2kModelID_len+1];
      /** \brief Max length of Software Code
       * Note that we reserve one extra char for null termination  */
      char N2kSwCode[Max_N2kSwCode_len+1];
      /** \brief Max length of Model Version
       * Note that we reserve one extra char for null termination */
      char N2kModelVersion[Max_N2kModelVersion_len+1];
      /** \brief Max length of Serial Code
       * Note that we reserve one extra char for null termination */
      char N2kModelSerialCode[Max_N2kModelSerialCode_len+1];
      /** \brief Certification level of the device*/
      unsigned char CertificationLevel;
      /** \brief Load Equivalency of the device
       * A Load Equivalence Number express the amount of current that
       * is drawn from an NMEA 2000 network. 1 equals to 50mA. If 
       * a device draws 151mA of current from the network, then its 
       * LEN is 4
      */
      unsigned char LoadEquivalency;

      /********************************************************************//**
       * \brief Set all the product infomation data of the structure
       *
       * \param _ModelSerialCode        Manufacturer's Model serial code,
       *                                default="". Max 32 chars. 
       * \param _ProductCode            Manufacturer's product code,
       *                                default=666
       * \param _ModelID                Manufacturer's  Model ID,
       *                                default="". Max 33 chars
       * \param _SwCode                 Manufacturer's software version code,
       *                                default="". Max 40 chars
       * \param _ModelVersion           Manufacturer's Model version
       *                                default="". Max 24 chars
       * \param _LoadEquivalency        Load equivalency ( x * 50mA, default=1)
       * \param _N2kVersion             N2k Standard version, default=2101
       * \param _CertificationLevel     Certification level, default=0
       */
      void Set(const char *_ModelSerialCode, 
               unsigned short _ProductCode=0xffff,  
               const char *_ModelID=0, 
               const char *_SwCode=0, 
               const char *_ModelVersion=0, 
               unsigned char _LoadEquivalency=0xff,  
               unsigned short _N2kVersion=0xffff, 
               unsigned char _CertificationLevel=0xff 
              ) {
        N2kVersion=(_N2kVersion!=0xffff?_N2kVersion:2101);
        ProductCode=_ProductCode;
        ClearSetCharBuf(_ModelID,sizeof(N2kModelID),N2kModelID);
        ClearSetCharBuf(_SwCode,sizeof(N2kSwCode),N2kSwCode);
        ClearSetCharBuf(_ModelVersion,sizeof(N2kModelVersion),N2kModelVersion);
        ClearSetCharBuf(_ModelSerialCode,sizeof(N2kModelSerialCode),N2kModelSerialCode);
        CertificationLevel=(_CertificationLevel!=0xff?_CertificationLevel:0);
        LoadEquivalency=(_LoadEquivalency!=0xff?_LoadEquivalency:1);
      }
      /*******************************************************************//**
       * \brief Clears out all data  */
      void Clear();
      /*******************************************************************//**
       * \brief Compares two product information structures
       *
       * \param Other An other product information structure
       * \return true 
       * \return false 
       */
      bool IsSame(const tProductInformation &Other);
  };
  /**********************************************************************//**
   * \class tDeviceInformation
   * \brief Class that holds all the device informations and several 
   *        helper functions to that
   */
  class tDeviceInformation {
  protected:
    /********************************************************************//**
     * \union   tUnionDeviceInformation
     * \brief   Union that holds the device informations
     *
     */
    typedef union {
      /** \brief Devicename */
      uint64_t Name;
      /******************************************************************//**
     
       * \brief Structure for device information
       */
      struct {
        /** \brief  32 bit number carrying Unique Number and Manufacturer 
         * Code
         * 
         * ManufacturerCode 11 bits , UniqueNumber 21 bits
         */
        uint32_t UnicNumberAndManCode; // 
        /** \brief  Device instance number */
        unsigned char DeviceInstance;
        /** \brief  Device function code 
         * 
         * see for Details: [NMEA2000 Device and Function Codes](https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf)
         * 
        */
        unsigned char DeviceFunction;
        /** \brief  Device class 
         * 
         * see for Details: [NMEA2000 Device and Function Codes](https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf)
         * 
         */
        unsigned char DeviceClass;
         
        /** \brief  Industrie Group and System Instance (each 4bits)
         * 
         * I found document: 
         * http://www.novatel.com/assets/Documents/Bulletins/apn050.pdf it 
         * says about next fields:  
         * The System Instance Field can be utilized to facilitate multiple 
         * NMEA 2000 networks on these larger marine platforms. NMEA 2000 
         * devices behind a bridge, router, gateway, or as part of some 
         * network segment could all indicate this by use and application 
         * of the System Instance Field. DeviceInstance and SystemInstance 
         * fields can be now changed by function 
         * \ref SetDeviceInformationInstances or by NMEA 2000 group 
         * function. Group function handling is build in the library.
        */
        unsigned char IndustryGroupAndSystemInstance; 
      };
    } tUnionDeviceInformation;

    /** \brief Union that contains all the Device Information  */
    tUnionDeviceInformation DeviceInformation;

  public:
    /*******************************************************************//**
     * \brief Construct a new empty Device Information object
     */
    tDeviceInformation() { DeviceInformation.Name=0; }

    /*******************************************************************//**
     * \brief Set a unique Number to the Device Information
     * \param _UniqueNumber   a unique number for the device (max 21.bits)
     */
    void SetUniqueNumber(uint32_t _UniqueNumber) { DeviceInformation.UnicNumberAndManCode=(DeviceInformation.UnicNumberAndManCode&0xffe00000) | (_UniqueNumber&0x1fffff); }

    /*******************************************************************//**
     * \brief Get the unique Number from the Device Information
     * \return uint32_t 
     */
    uint32_t GetUniqueNumber() const { return DeviceInformation.UnicNumberAndManCode&0x1fffff; }
    
    /*******************************************************************//**
     * \brief Set the Manufacturer Code to the Device Information
     * \param _ManufacturerCode Manufacturer Code (max 11bits)
     */
    void SetManufacturerCode(uint16_t _ManufacturerCode) { DeviceInformation.UnicNumberAndManCode=(DeviceInformation.UnicNumberAndManCode&0x1fffff) | (((unsigned long)(_ManufacturerCode&0x7ff))<<21); }

    /*******************************************************************//**
     * \brief Get the Manufacturer Code from the Device Information
     * \return uint16_t 
     */
    uint16_t GetManufacturerCode() const { return DeviceInformation.UnicNumberAndManCode>>21; }

    /*******************************************************************//**
     * \brief Set the Device Instance to the Device Information
     * \param _DeviceInstance   Instance for the device
     */
    void SetDeviceInstance(unsigned char _DeviceInstance) { DeviceInformation.DeviceInstance=_DeviceInstance; }

    /*******************************************************************//**
     * \brief Get the Device Instance from the Device Information
     * \return unsigned char 
     */
    unsigned char GetDeviceInstance() const { return DeviceInformation.DeviceInstance; }

    /*******************************************************************//**
     * \brief Get the Device Instance (lower bits) from the Device Information
     * \return unsigned char 
     */
    unsigned char GetDeviceInstanceLower() const { return DeviceInformation.DeviceInstance & 0x07; }
    /*******************************************************************//**
     * \brief Get the Device Instance (upper bits) from the Device Information
     * \return unsigned char 
     */
    unsigned char GetDeviceInstanceUpper() const { return (DeviceInformation.DeviceInstance>>3) & 0x1f; }

    /*******************************************************************//**
     * \brief Set the Device Function to the Device Information
     * \param _DeviceFunction   Device function code, \ref 
     * tDeviceInformation::tUnionDeviceInformation::DeviceFunction
     */
    void SetDeviceFunction(unsigned char _DeviceFunction) { DeviceInformation.DeviceFunction=_DeviceFunction; }

    /*******************************************************************//**
     * \brief Get the Device Function from the Device Information
     * \return  unsigned char ->  Device function code, \ref 
     * tDeviceInformation::tUnionDeviceInformation::DeviceFunction
     */
    unsigned char GetDeviceFunction() const { return DeviceInformation.DeviceFunction; }
    
    /*******************************************************************//**
     * \brief Set the Device Class to the Device Information
     * \param _DeviceClass   Device class code, \ref 
     * tDeviceInformation::tUnionDeviceInformation::DeviceClass
     */
    void SetDeviceClass(unsigned char _DeviceClass) { DeviceInformation.DeviceClass=((_DeviceClass&0x7f)<<1); }
        
    /*******************************************************************//**
     * \brief Get the Device Class from the Device Information
     * \return  unsigned char ->  Device class code, \ref 
     * tDeviceInformation::tUnionDeviceInformation::DeviceClass
     */
    unsigned char GetDeviceClass() const { return DeviceInformation.DeviceClass>>1; }

    /********************************************************************//**
     * \brief Set the Industry Group to the Device Information
     * \param _IndustryGroup    Industry Group
     */
    void SetIndustryGroup(unsigned char _IndustryGroup) { DeviceInformation.IndustryGroupAndSystemInstance=(DeviceInformation.IndustryGroupAndSystemInstance&0x0f) | (_IndustryGroup<<4) | 0x80; }
    
    /********************************************************************//**
     * \brief Get the Industry Group from the Device Information
     * \return unsigned char 
     */
    unsigned char GetIndustryGroup() const { return (DeviceInformation.IndustryGroupAndSystemInstance>>4) & 0x07; }

    /********************************************************************//**
     * \brief Set the System Instance to the Device Information
     * \param _SystemInstance    System Instance 
     */
    void SetSystemInstance(unsigned char _SystemInstance) { DeviceInformation.IndustryGroupAndSystemInstance=(DeviceInformation.IndustryGroupAndSystemInstance&0xf0) | (_SystemInstance&0x0f); }
    
    /********************************************************************//**
     * \brief Get the System Instance from the Device Information
     * \return unsigned char 
     */
    unsigned char GetSystemInstance() const { return DeviceInformation.IndustryGroupAndSystemInstance&0x0f; }
    /*** ****************************************************************//**
     * \brief Get the Name from the Device Information
     * \return uint64_t 
     */
    uint64_t GetName() const { return DeviceInformation.Name; }
    /********************************************************************//**
     * \brief Set the Name to the Device Information
     * \param _Name  Nmae of the device
     */
    void SetName(uint64_t _Name) { DeviceInformation.Name=_Name; }
    /********************************************************************//**
     * \brief Check if two devices are the same, by comparing the device name
     * \param Other Name of th other device
     * \return true 
     * \return false 
     */
    inline bool IsSame(uint64_t Other) { return GetName()==Other; }
  };
  /************************************************************************//**
   * \class tDevice
   * \brief This class represents a N2k device 
   *
   */
  class tDevice {
    protected:
      /** \brief This object holds all necessary device informations*/
      tDeviceInformation DevI;
      /** \brief Timestamp when this device was created*/
      unsigned long CreateTime;
      /** \brief Source address on bus for this device*/
      uint8_t Source;
      
    public:
      /*******************************************************************//**
       * \brief Construct a new Device object
       *
       * Initialize all the attributes of the device. 
       * 
       * \param _Name   Name of the device
       * \param _Source Source address on the bus (default = 255)
       */
      tDevice(uint64_t _Name, uint8_t _Source=255) { Source=_Source; DevI.SetName(_Name); CreateTime=N2kMillis(); }
      /*******************************************************************//**
       * \brief Destroy the Device object */
      virtual ~tDevice() {;}

      /** \brief Returns the Source Address of this device*/
      uint8_t GetSource() const { return Source; }
      /** \brief Returns the Time of Creation of this device*/
      unsigned long GetCreateTime() const { return CreateTime; }

      /******************************************************************//**
       * \brief Get the Name of this device
       * \return uint64_t 
       */
      inline uint64_t GetName() const { return DevI.GetName(); }
      /******************************************************************//**
      * \brief Check if two devices are the same, by comparing the device name
      * \param Other Name of th other device
      * \return true 
      * \return false 
      */
      inline bool IsSame(uint64_t Other) { return DevI.IsSame(Other); }
      /*******************************************************************//**
       * \brief Get the unique Number from the Device Information
       * \return uint32_t 
       */
      inline uint32_t GetUniqueNumber() const { return DevI.GetUniqueNumber(); }
      /*******************************************************************//**
      * \brief Get the Manufacturer Code from the Device Information
      * \return uint16_t 
      */
      inline uint16_t GetManufacturerCode() const { return DevI.
      GetManufacturerCode(); }
      /*******************************************************************//**
       * \brief Get the Device Instance from the Device Information
       * \return unsigned char 
       */
      inline unsigned char GetDeviceInstance() const { return DevI.GetDeviceInstance(); }
      /*******************************************************************//**
       * \brief Get the Device Instance (lower bits) from the Device Information
       * \return unsigned char 
       */
      inline unsigned char GetDeviceInstanceLower() const { return DevI.GetDeviceInstanceLower(); }
      /*******************************************************************//**
       * \brief Get the Device Instance (upper bits) from the Device Information
       * \return unsigned char 
       */
      inline unsigned char GetDeviceInstanceUpper() const { return DevI.GetDeviceInstanceUpper(); }
      /*******************************************************************//**
       * \brief Get the Device Function from the Device Information
       * \return  unsigned char ->  Device function code, \ref 
       * tDeviceInformation::tUnionDeviceInformation::DeviceFunction
       */
      inline unsigned char GetDeviceFunction() const { return DevI.GetDeviceFunction(); }
      /*******************************************************************//**
       * \brief Get the Device Class from the Device Information
       * \return  unsigned char ->  Device class code, \ref 
       * tDeviceInformation::tUnionDeviceInformation::DeviceClass
       */
      inline unsigned char GetDeviceClass() const { return DevI.GetDeviceClass(); }
      /********************************************************************//**
       * \brief Get the Industry Group from the Device Information
       * \return unsigned char 
       */
      inline unsigned char GetIndustryGroup() const { return DevI.GetIndustryGroup(); }
      /********************************************************************//**
       * \brief Get the System Instance from the Device Information
       * \return unsigned char 
       */
      inline unsigned char GetSystemInstance() const { return DevI.GetSystemInstance(); }

      // Product information
      /** \brief Get N2k Standard version  from the product information 
       * of this device*/
      virtual unsigned short GetN2kVersion() const=0;
      /** \brief Get the product code from the product information of 
       * this device*/
      virtual unsigned short GetProductCode() const=0;
      /** \brief Get the model ID  from the product information of this device*/
      virtual const char * GetModelID() const=0;
      /** \brief Get the Software version code from the product 
       * information of this device*/
      virtual const char * GetSwCode() const=0;
      /** \brief Get the model version from the product information 
       * of this device*/
      virtual const char * GetModelVersion() const=0;
      /** \brief Get the model serial code from the product information 
       * of this device*/
      virtual const char * GetModelSerialCode() const=0;
      /** \brief Get the certification level from the product information 
       * of this device*/
      virtual unsigned short GetCertificationLevel() const=0;
      /** \brief Get the load equivalency from the product information 
       * of this device*/
      virtual unsigned short GetLoadEquivalency() const=0;

      // Configuration information
      /** \brief Get the installation description 1 from the configuration 
       * information of this device*/
      virtual const char * GetInstallationDescription1() const { return 0; }
      /** \brief Get the installation description 2 from the configuration 
       * information of this device*/
      virtual const char * GetInstallationDescription2() const { return 0; }

      /** \brief Get the list of transmitted PGNs from this device*/
      virtual const unsigned long * GetTransmitPGNs() const { return 0; }
      /** \brief Get the list of received PGNs from this device*/
      virtual const unsigned long * GetReceivePGNs() const { return 0; }
  };
  /************************************************************************//**
   * \class tMsgHandler
   * \brief Message handler class
   *
   */
  class tMsgHandler {
    private:
      friend class tNMEA2000;
      /** \brief Pointer to a message handler*/
      tMsgHandler *pNext;
      /** \brief Pointer to a tNMEA2000 object*/
      tNMEA2000 *pNMEA2000;
    protected:
      /** Parameter Group Number*/
      unsigned long PGN;
      /*******************************************************************//**
       * \brief Handles a given message       *
       * \param N2kMsg Reference to a N2kMsg Object
       */
      virtual void HandleMsg(const tN2kMsg &N2kMsg)=0;
      /** \brief Returns the tNMEA2000 object of this handler
       * \return  tNMEA2000   */
      tNMEA2000 *GetNMEA2000() { return pNMEA2000; }
    public:
      /******************************************************************//**
       * \brief Construct a new Message Handler object
       *
       * Attaches this message handler to a tNMEA2000 object.
       * 
       * \param _PGN        PGN of the message that should be handled
       * \param _pNMEA2000  Pointer to tNMEA2000 object, where the handle 
       *                    should be attached
       */
      tMsgHandler(unsigned long _PGN=0, tNMEA2000 *_pNMEA2000=0) {
        PGN=_PGN; pNext=0; pNMEA2000=0;
        if ( _pNMEA2000!=0 ) _pNMEA2000->AttachMsgHandler(this);
      }
      /*******************************************************************//**
       * \brief Destroys the Message Handler object
       */
      virtual ~tMsgHandler() { if ( pNMEA2000!=0 ) pNMEA2000->DetachMsgHandler(this); }
      /*******************************************************************//**
       * \brief Return the PGN that is handled by this message handler
       * \return unsigned long 
       */
      inline unsigned long GetPGN() const { return PGN; }
  };

public:
  /************************************************************************//**
   * \enum    tForwardType
   * \brief   Type how to forward messages in listen mode
   */
  typedef enum { 
            /** Forwards messages to output port in Actisense format. Note 
             * that some Navigation sw uses this. */
            fwdt_Actisense,

            /** Forwards messages to output port in clear text. This is e.g. 
             *  for debugging. */
            fwdt_Text 
          } tForwardType;
  
  /************************************************************************//**
   * \enum    tN2kMode
   * \brief   System mode. Meaning how the device will behave on the 
   *          NMEA2000 bus
   */
  typedef enum { 
            /** Default mode. Listen bus and forwards messages to default 
             * port in Actisense format. You can not send any data to the bus.
            */
            N2km_ListenOnly, 
            /** This is for devices, which only sends data to the bus e.g. 
             * RPM or temperature monitor. Remember to set right device 
             * information first.
            */
            N2km_NodeOnly, 
            /** In this mode, device can be e.g. temperature monitor and 
             * as N2km_ListenOnly.
            */
            N2km_ListenAndNode, 
            /**  Only for message sending. Device will not inform itself 
             * to the bus. Messages will not be forwarded to the stream.  
             * By setting message handler, you can still read messages 
             * and handle them by yourself.
            */
            N2km_SendOnly,
            /** Listen bus and forwards messages to default port in 
             * Actisense format. Messages can be send. Device will not 
             * inform itself to the bus.
            */
            N2km_ListenAndSend 
            } tN2kMode;

  /************************************************************************//**
   * \enum    tDebugMode
   * \brief   For debugging we have some cases for SendMsg
   */
  typedef enum {
            /** Directs data to CAN bus
            */
            dm_None, 
            /** Directs sended data to serial in clear text format
            */
            dm_ClearText, 
            /** Directs sended data to serial as Actisense format.
            */
            dm_Actisense,  
          } tDebugMode;

  /************************************************************************//**
   * \struct   tConfigurationInformation
   * \brief   Structure that holds the Configuration Information of this device
   */
  struct tConfigurationInformation {
      /** \brief pointer to char array holding the manufacturer information */
      const char *ManufacturerInformation;
      /** \brief pointer to char array holding the installation description 1 */
      const char *InstallationDescription1;
      /** \brief pointer to char array holding the installation description 2 */
      const char *InstallationDescription2;
  };

protected:

  /************************************************************************//**
   * \enum    tOpenState
   * \brief   State of ...
   *
   * \todo Finalize Description....
   */
  typedef enum {
                  os_None			  ///< State none
                  ,os_OpenCAN		///< State Open CAN
                  ,os_WaitOpen	///< State Wait Open
                  ,os_Open			///< State Open
               } tOpenState;

  /************************************************************************//**
   * \class   tInternalDevice
   * \brief   This class represents an internal device
   *
   */
  class tInternalDevice {
  public:
    /** \brief Source address of this device on the NMEA2000 bus*/
    uint8_t N2kSource;
    /** \brief This holds all the Device Informations for this specific device*/
    tDeviceInformation DeviceInformation;
    // Product information
    /** \brief This holds all the Product Informations for this 
     * specific device*/
    const tProductInformation *ProductInformation;
    /** \brief This holds all the local (???) Product Informations for this 
     * specific device*/
    tProductInformation *LocalProductInformation;
    /** \brief This holds the Manufacturer Code for this 
     * specific device*/
    char *ManufacturerSerialCode;
    /** \brief Timestamp set while last \ref tNMEA2000::SendIsoAddressClaim 
     * was executed*/
    tN2kScheduler PendingIsoAddressClaim;
    /** \brief Timestamp set while last \ref tNMEA2000::SendProductInformation 
     * was executed*/
    tN2kScheduler PendingProductInformation;
    /** \brief Timestamp set while last \ref 
     * tNMEA2000::SendConfigurationInformation was executed */
    tN2kScheduler PendingConfigurationInformation;
    /** \brief Timer value for AddressClaim
    */
    tN2kScheduler AddressClaimTimer;
    /** \brief Pointer to a buffer tha holds all supported transmit 
     * PGNs for this device*/
    const unsigned long *TransmitMessages;
    /** \brief Pointer to a buffer tha holds all supported receive 
     * PGNs for this device*/
    const unsigned long *ReceiveMessages;
    /** \brief Fast packet PGNs sequence counters*/
    unsigned long *PGNSequenceCounters;
    /** \brief Fast packet PGNs sequence counters*/
    size_t MaxPGNSequenceCounters;
    /** \brief Holds the highest source address for Address Claim process*/
    uint8_t AddressClaimEndSource;
    /** \brief internal device has pending information*/
    bool HasPendingInformation;

#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
      /** \brief Pending N2k message */
      tN2kMsg PendingTPMsg;
      /** \brief Timestamp, when next data packet can be send on TP broadcast*/
      tN2kScheduler NextDTSendTime; 
      /** \brief Next Sequence*/
      uint8_t NextDTSequence;
#endif
#if !defined(N2K_NO_HEARTBEAT_SUPPORT)
	/** \brief Intervall for Heartbeat */
    #define DefaultHeartbeatInterval 60000
    /** \brief Scheduler for the heartbeat message */
    tN2kSyncScheduler HeartbeatScheduler;
    /** \brief Heartbeat Sequence */
    uint8_t HeartbeatSequence;
#endif

  public:
    /********************************************************************//**
     * \brief Construct a new Internal Device object
     * 
     * Initialize all the attributes of this internal device.
     */
    tInternalDevice() {
      N2kSource=0;
      HasPendingInformation=false;
      ProductInformation=0; LocalProductInformation=0; ManufacturerSerialCode=0;
      AddressClaimEndSource=N2kMaxCanBusAddress; //GetNextAddressFromBeginning=true;
      TransmitMessages=0; ReceiveMessages=0;
      PGNSequenceCounters=0; MaxPGNSequenceCounters=0;
#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
      NextDTSequence=0;
#endif

#if !defined(N2K_NO_HEARTBEAT_SUPPORT)
      HeartbeatSequence=0;
#endif
    }
    /*********************************************************************//**
     * \brief Set the timestamp for Pending an Iso Address Claim message
     * \param FromNow   Variable time delay in ms
     * \sa tNMEA2000::SendIsoAddressClaim 
     */
    void SetPendingIsoAddressClaim(unsigned long FromNow=2) { PendingIsoAddressClaim.FromNow(FromNow); HasPendingInformation=true; }
    /*********************************************************************//**
     * \brief Check if enough time has passed 
     * 
     * Checks if enough time has passed since \ref 
     * SetPendingIsoAddressClaim, so that \ref SendIsoAddressClaim can 
     * be executed.
     * 
     * \return true   enough time has passed
     * \return false 
     */
    bool QueryPendingIsoAddressClaim() { return PendingIsoAddressClaim.IsTime(); }
    /** \brief Resets \ref PendingIsoAddressClaim to zero*/
    void ClearPendingIsoAddressClaim() { PendingIsoAddressClaim.Disable(); UpdateHasPendingInformation(); }

    /*********************************************************************//**
     * \brief Set the timestamp for Pending a ProductInformation message
     *
     * This function use strange delays to avoid synchronization of many 
     * messages.
     * \sa tNMEA2000::SendProductInformation 
     */
    void SetPendingProductInformation() { PendingProductInformation.FromNow(187+N2kSource*8); HasPendingInformation=true; } // Use strange increment to avoid synchronize
    /** \brief Resets \ref PendingProductInformation to zero*/
    void ClearPendingProductInformation() { PendingProductInformation.Disable(); UpdateHasPendingInformation(); }
    /*********************************************************************//**
     * \brief Check if enough time has passed 
     * 
     * Checks if enough time has passed since \ref 
     * SetPendingProductInformation, so that \ref SendProductInformation can 
     * be executed.
     * 
     * \return true   enough time has passed
     * \return false 
     */
    bool QueryPendingProductInformation() { return PendingProductInformation.IsTime(); }

    /*********************************************************************//**
     * \brief Set the timestamp for Pending a ConfigurationInformation message
     *
     * This function use strange delays to avoid synchronization of many 
     * messages.
     * \sa tNMEA2000::SendConfigurationInformation 
     */
    void SetPendingConfigurationInformation() { PendingConfigurationInformation.FromNow(187+N2kSource*10); HasPendingInformation=true; } // Use strange increment to avoid synchronize
    /** \brief Resets \ref PendingConfigurationInformation to zero*/
    void ClearPendingConfigurationInformation() { PendingConfigurationInformation.Disable(); UpdateHasPendingInformation(); }
    /*********************************************************************//**
     * \brief Check if enough time has passed 
     * 
     * Checks if enough time has passed since \ref 
     * SetPendingConfigurationInformation, so that \ref 
     * SendConfigurationInformation can be executed.
     * 
     * \return true   enough time has passed
     * \return false 
     */
    bool QueryPendingConfigurationInformation() { return PendingConfigurationInformation.IsTime(); }
    /*********************************************************************//**
     * \brief Updates \ref AddressClaimEndSource 
     */
    void UpdateAddressClaimEndSource() {
      AddressClaimEndSource=N2kSource;
      if ( AddressClaimEndSource>0 ) { AddressClaimEndSource--; } else { AddressClaimEndSource=N2kMaxCanBusAddress; }
    }
    void UpdateHasPendingInformation() {
      HasPendingInformation=  PendingIsoAddressClaim.IsEnabled()
                            | PendingProductInformation.IsEnabled()
                            | PendingConfigurationInformation.IsEnabled()
                            #if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
                            | NextDTSendTime.IsEnabled()
                            #endif
                            ;
    }
  };

protected:
  
  /** \brief Forward mode bit: ->  If set, forward is enabled */
  static const int FwdModeBit_EnableForward        = BIT(0); 
  /** \brief Forward mode bit: ->  System messages will be forwarded */
  static const int FwdModeBit_SystemMessages       = BIT(1); 
  /** \brief Forward mode bit: ->  Only known messages will be forwarded. 
   * System messages will be forwarded according its own bit*/
  static const int FwdModeBit_OnlyKnownMessages    = BIT(2); 
  /** \brief Forward mode bit: ->  Forward also all messages, 
   * what this device will send */
  static const int FwdModeBit_OwnMessages          = BIT(3); 
  /** \brief Forward mode bit: ->  Only known messages will be handled */
  static const int HandleModeBit_OnlyKnownMessages = BIT(4); 

protected:
    /** \brief  Attribute that holds the actual Debug Mode (default = md_none)*/
    tDebugMode dbMode;
    /** \brief  Actual operation mode of this device (default = 
     * N2km_ListenOnly)
     */
    tN2kMode N2kMode;
    /** \brief  Actual message forward type (default = fwdt_Actisense)*/
    tForwardType ForwardType; 
    /** \brief  Actual message forward operation mode 
     *  (default = all messages - also system and own)*/
    unsigned int ForwardMode; 
    /** \brief  Actual stream to be used for forward messaging*/
    N2kStream *ForwardStream;
    /** \brief  Pointer to a buffer for Message Handlers*/
    tMsgHandler *MsgHandlers;

    /** Open the Scheduler */
    tN2kScheduler OpenScheduler;
    /** State of the .... */
    tOpenState OpenState;
    /** \brief  Flag that the address has changed */
    bool AddressChanged;
    /** \brief  Flag that the device information has changed */
    bool DeviceInformationChanged;

    /** \brief  Pointer to a buffer for all internal devices */
    tInternalDevice *Devices;
    /** \brief  Number of devices */
    int DeviceCount;
//    unsigned long N2kSource[Max_N2kDevices];

    /** \brief  Pointer to a buffer for local Configuration Information*/
    char *LocalConfigurationInformationData;
    /** \brief Configuration Information of the device*/
    tConfigurationInformation ConfigurationInformation;

    const unsigned long *SingleFrameMessages[N2kMessageGroups];
    const unsigned long *FastPacketMessages[N2kMessageGroups];

    /*********************************************************************//**
     * \struct  tCANSendFrame
     * \brief   Structure holds all the data needed for a valide CAN-Message
     */
    class tCANSendFrame
    {
    public:
      /** \brief  ID of the CAN Message*/
      unsigned long id;
      /** \brief  Length of carried data of the CAN Message*/
      unsigned char len;
      /** \brief  Data payload for the CAN Message*/
      unsigned char buf[8];
      /** \brief  Has the CAN Message to wait before sending*/
      bool wait_sent;

    public:
      /** Clears all the fieds of the CAN Message */
      void Clear() {id=0; len=0; for (int i=0; i<8; i++) { buf[i]=0; } }
    };

protected:
    /** \brief Buffer for received messages */
    tN2kCANMsg *N2kCANMsgBuf;
    /** \brief Max number CAN messages that can go to the buffer 
     *          \ref N2kCANMsgBuf */
    uint8_t MaxN2kCANMsgs;

    /** \brief Buffer for ssend out CAN messages */
    tCANSendFrame *CANSendFrameBuf;
    /** \brief Max number of send out CAN messages that can go to the buffer 
     *          \ref CANSendFrameBuf 
     * \sa \ref InitCANFrameBuffers()*/
    uint16_t MaxCANSendFrames;
    /** \brief  Index for the CAN message Send buffer 
     * \todo Please double check Docu
     */
    uint16_t CANSendFrameBufferWrite;
    /** \brief  Index for the CAN message Send buffer
     * \todo Please double check Docu
     */
    uint16_t CANSendFrameBufferRead;
    /** \brief Max number received CAN messages that can go to the buffer 
     * \sa \ref InitCANFrameBuffers()
    */
    uint16_t MaxCANReceiveFrames;

    /** \brief ???
    * \todo Better documentation needed */
    void (*OnOpen)();
        
    /** \brief Handler callbacks for normal messages */
    void (*MsgHandler)(const tN2kMsg &N2kMsg);  
    /** \brief Handler callbacks for 'ISORequest' messages */
    bool (*ISORqstHandler)(unsigned long RequestedPGN, unsigned char Requester, int DeviceIndex);

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)
    /** \brief Pointer to Buffer for GRoup Function Handlers*/
    tN2kGroupFunctionHandler *pGroupFunctionHandlers;
#endif

protected:
    /*********************************************************************//**
     * \brief Send a CAN Frame
     * 
     * This Virtual function will be overridden by a derived class for 
     * specific interfaces according to the hardware which is used. 
     * Currently there are own classes like NMEA2000_Teensyx, NMEA2000_teensy,
     * NMEA2000_esp32, NMEA2000_due, NMEA2000_mcp, NMEA2000_avr, NMEA2000_mbed
     * and NMEA2000_socketCAN.
     * 
     * \sa \ref secHWlib
     * 
     * \param id        ID of the CAN frame
     * \param len       length of payload for the message
     * \param buf       buffer with the payload
     * \param wait_sent   Has the message to wait before sending
     * 
     * \return true   -> Success
     * \return false  -> there is no space in the queue
    */
    virtual bool CANSendFrame(unsigned long id, unsigned char len, const unsigned char *buf, bool wait_sent=true)=0;
    /*********************************************************************//**
     * \brief Open the CAN Interface
     * 
     * This Virtual function will be overridden by a derived class for 
     * specific interfaces according to the hardware which is used. 
     * Currently there are own classes like NMEA2000_Teensyx, NMEA2000_teensy,
     * NMEA2000_esp32, NMEA2000_due, NMEA2000_mcp, NMEA2000_avr, NMEA2000_mbed
     * and NMEA2000_socketCAN.
     * 
     * \sa \ref secHWlib
     * 
     * \return true   -> Success
     * \return false  -> currently prevent accidental by second instance. 
     *                   Maybe possible in future.
     */

    virtual bool CANOpen()=0;
    /*********************************************************************//**
     * \brief Get a CAN Frame
     * 
     * This Virtual function will be overridden by a derived class for 
     * specific interfaces according to the hardware which is used. 
     * Currently there are own classes like NMEA2000_Teensyx, NMEA2000_teensy,
     * NMEA2000_esp32, NMEA2000_due, NMEA2000_mcp, NMEA2000_avr, NMEA2000_mbed
     * and NMEA2000_socketCAN.
     * 
     * \sa \ref secHWlib
     * 
     * \return true   -> Ther is a new frame
     * \return false  -> 
     */
    virtual bool CANGetFrame(unsigned long &id, unsigned char &len, unsigned char *buf)=0;

    
    /*********************************************************************//**
     * \brief Initialize CAN Frame buffers
     *
     * This will be called on \ref tNMEA2000::Open() before any other 
     * initialization. Inherit this, if buffers can be set for the driver
     * and you want to change size of library send frame buffer size. See e.g. 
     * NMEA2000_teensy.cpp.
     */
    virtual void InitCANFrameBuffers();
#if defined(DEBUG_NMEA2000_ISR)
    virtual void TestISR() {;}
#endif

protected:
    /**********************************************************************//**
     * \brief Sends pending all frames
     *
     * \return true   -> Success
     * \return false  -> Message could not be send out
     */
    bool SendFrames();
    /**********************************************************************//**
     * \brief Sends a single CAN frame
     * 
     * This function sends a CAN Message to the buffer, if we can not sent 
     * frame immediately via \ref CANSendFrame()
     * 
     * \param id {type} 
     * \param len {type} 
     * \param buf {type} 
     * \param wait_sent {type} 
     * 
     * \return true   -> success
     * \return false  -> failed
     */
    bool SendFrame(unsigned long id, unsigned char len, const unsigned char *buf, bool wait_sent=true);

    /*********************************************************************//**
     * \brief Get the Next Free CAN Frame  from \ref CANSendFrameBuf
     * \return tCANSendFrame* 
     */
    tCANSendFrame *GetNextFreeCANSendFrame();

    /*********************************************************************//**
     * \brief Send ISO AddressClaim, Product Information and Config 
     *        Information
     *  
     * Currently Product Information and Configuration Information will we 
     * pended on ISO request. This is because specially for broadcasted 
     * response it may take a while, when higher priority devices sends 
     * their response.
     * \sa  \ref SendIsoAddressClaim(), \ref SendProductInformation(),
     *      \ref SendConfigurationInformation()
     */
    void SendPendingInformation();

protected:
    /*********************************************************************//**
     * \brief Initialize all devices
     * 
     * This function initializes the array \ref Devices with the size of 
     * \ref DeviceCount and sets default device information.
     * 
     * Default Device Info is:
     *  - Unique ID = Device Index +1 ==> 21 bit resolution, max 2097151. Each 
     *    device from same manufacturer should have unique number.
     *  - Device Function = 130 ==> PC Gateway 
     *  - Device Class = 25  ==> Inter/Intranetwork Device. 
     *  - Manufacturer Code = 2046  ==> Maximum 2046
     *  - Industrie Group Code = 4  ==> Marine
     * 
     * \sa
     *  - https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
     *  - https://web.archive.org/web/20190529161431/http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
     */
    void InitDevices();
    /*********************************************************************//**
     * \brief Determines if the CAN BUS is already initialized
     * \return true 
     * \return false 
     */
    bool IsInitialized() { return (N2kCANMsgBuf!=0); }

#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
    /*********************************************************************//**
     * \brief Find index for free space for a message on \ref N2kCANMsgBuf
     *
     * This functions searches for free space at \ref N2kCANMsgBuf. If 
     * there is free space in buffer or there is a message with the 
     * same, source, PGN and destination at the buffer the corresponding
     * Index is returned. If this is not the case, the Index of the oldest
     * message inside the buffer is returned.
     * 
     * \param PGN           PNG for the message
     * \param Source        Source address of the message 
     * \param Destination   Destination of the message
     * \param TPMsg         Message is Multi Packet message
     * \param MsgIndex      Index 
     */
    void FindFreeCANMsgIndex(unsigned long PGN, unsigned char Source, unsigned char Destination, bool TPMsg, uint8_t &MsgIndex);
#else
    /*********************************************************************//**
     * \brief Find index for free space for a message on \ref N2kCANMsgBuf
     *
     * This functions searches for free space at \ref N2kCANMsgBuf. If 
     * there is free space in buffer or there is a message with the 
     * same, source, PGN and destination at the buffer the corresponding
     * Index is returned. If this is not the case, the Index of the oldest
     * message inside the buffer is returned.
     * 
     * \param PGN           PNG for the message
     * \param Source        Source address of the message 
     * \param Destination   Destination of the message
     * \param MsgIndex      Index 
     */
    void FindFreeCANMsgIndex(unsigned long PGN, unsigned char Source, unsigned char Destination, uint8_t &MsgIndex);
#endif
    /*********************************************************************//**
     * \brief Function handles received CAN frame and adds it to tN2kCANMsg
     *  
     * This function returns Index to a ready \ref tN2kCANMsg on buffer
     * \ref N2kCANMsgBuf (max \ref MaxN2kCANMsgs), 
     * if we skipped the frame or message is not ready (fast packet or 
     * ISO Multi-Packet)
     * 
     * \param canId     ID of CAN message
     * \param len       length of payload
     * \param buf       buffer for payload of message
     * \return uint8_t  -> Index of the CAN message on \ref N2kCANMsgBuf
     */
    uint8_t SetN2kCANBufMsg(unsigned long canId, unsigned char len, unsigned char *buf);

    /*********************************************************************//**
     * \brief Check if this PNG is a fast packet message
     * 
     * Determines if this given PGN belongs to a fast packet message by
     * checking if a corresponding message is listed on \ref
     * FastPacketMessages[]
     * 
     * \param PGN     PGN to be checked
     * \return true 
     * \return false 
     */
    bool IsFastPacketPGN(unsigned long PGN);

    /*********************************************************************//**
     * \brief Check if this PNG is a fast packet message
     * 
     * Determines if this given PGN belongs to a fast packet message by
     * checking if a corresponding message is listed on \ref
     * FastPacketMessages[]
     * 
     * \param N2kMsg Reference to a N2kMsg Object
     * \return true 
     * \return false 
     */
    bool IsFastPacket(const tN2kMsg &N2kMsg);

    /*********************************************************************//**
     * \brief Check if this Message is known to the system
     * 
     * Determines wether this message is known to the system, either by beeing
     * a default, mandatory or system message or this specific message which is
     * listed in \ref SingleFrameMessages \\ \ref FastPacketMessages.
     *
     * \sa \ref IsDefaultSingleFrameMessage, \ref IsMandatoryFastPacketMessage, 
     *      \ref IsDefaultFastPacketMessage, \ref IsSingleFrameSystemMessage, 
     *      \ref IsFastPacketSystemMessage, 
     * 
     * \param PGN             PGN to be checked
     * \param SystemMessage   Flag system message
     * \param FastPacket      Flag fast packet
     * \return true 
     * \return false 
     */
    bool CheckKnownMessage(unsigned long PGN, bool &SystemMessage, bool &FastPacket);

    /*********************************************************************//**
     * \brief Handles a received system message
     *  
     * If the node is not \ref N2km_SendOnly or \ref N2km_ListenAndSend this
     * function chooses the correct handler for the given system message.
     * 
     * \sa  \ref HandleISORequest,  \ref HandleISOAddressClaim,
     *      \ref HandleCommandedAddress, \ref HandleCommandedAddress
     * 
     * \param MsgIndex  Message Index on \ref N2kCANMsgBuf
     * \return true     -> message was handled
     * \return false 
     */
    bool HandleReceivedSystemMessage(int MsgIndex);

    /*********************************************************************//**
     * \brief Forwards a N2k message
     * 
     * This function forwards a n2k message according to the forward type 
     * \ref ForwardType( \ref  tForwardType) to the correct 
     * \ref ForwardStream.
     *
     * \param N2kMsg  N2k message object
     */
    void ForwardMessage(const tN2kMsg &N2kMsg);
    
    /*********************************************************************//**
     * \brief Forwards a N2k message
     * 
     * This function forwards a n2k message according to the forward type 
     * \ref ForwardType( \ref  tForwardType) to the correct 
     * \ref ForwardStream.
     *
     * \param N2kCanMsg  N2k CAN message object, see \ref tN2kCANMsg
     */
    void ForwardMessage(const tN2kCANMsg &N2kCanMsg);
    
    /*********************************************************************//**
     * \brief Respond to an ISO request
     *
     * If there is now IsoAdressClaim procedure started for this device, we
     * respond for the requested PGN with sending IsoAddressClaim, Tx/Rx PGN 
     * lists, Product- / Config informations or an user defined ISORqstHandler.
     * 
     * If non of this fits the RequestedPGN, we directly respond to the 
     * requester NAK. ( \ref SetN2kPGNISOAcknowledgement)
     * 
     * \param N2kMsg        Reference to a N2kMsg Object
     * \param RequestedPGN  Requested PGN
     * \param iDev          index of the device on \ref Devices
     */
    void RespondISORequest(const tN2kMsg &N2kMsg, unsigned long RequestedPGN, int iDev);

    /*********************************************************************//**
     * \brief Handles an Iso Request
     * 
     * The function determines if the request is for us (Broadcast message or
     * device in our list) and responds to the request.
     *
     * \param N2kMsg        Reference to a N2kMsg Object
     */
    void HandleISORequest(const tN2kMsg &N2kMsg);

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)

    /*********************************************************************//**
     * \brief Respond to an Group Function
     *
     * 
     * Document https://web.archive.org/web/20170609033039/http://www.nmea.org/Assets/20140109%20nmea-2000-corrigendum-tc201401031%20pgn%20126208.pdf
     * defines that systems should respond to NMEA Request/Command/Acknowledge 
     * group function PGN 126208. Here we first call callback and if that will 
     * not handle function, we use default handler.
     * 
     * \param N2kMsg        Reference to a N2kMsg Object
     * \param GroupFunctionCode 
     * \param PGNForGroupFunction 
     * \param iDev          index of the device on \ref Devices
     */
    void RespondGroupFunction(const tN2kMsg &N2kMsg, tN2kGroupFunctionCode GroupFunctionCode, unsigned long PGNForGroupFunction, int iDev);

    /*********************************************************************//**
     * \brief Handles a Group Function
     * 
     * Document https://web.archive.org/web/20170609033039/http://www.nmea.org/Assets/20140109%20nmea-2000-corrigendum-tc201401031%20pgn%20126208.pdf
     * defines that systems should respond to NMEA Request/Command/Acknowledge 
     * group function PGN 126208. On the document it is not clear can request 
     * be send as broadcast, so we handle it, if we can.
     *
     * \param N2kMsg        Reference to a N2kMsg Object
     */
    void HandleGroupFunction(const tN2kMsg &N2kMsg);
#endif

    /*********************************************************************//**
     * \brief Starting the ISO Address Claim for a device
     *
     * \param iDev          index of the device on \ref Devices
     */
    void StartAddressClaim(int iDev);

    /*********************************************************************//**
     * \brief Starting the ISO Address Claim for all devices
     *
     */
    void StartAddressClaim();

    /*********************************************************************//**
     * \brief Checks if the IsoAddressClaim is already started
     * 
     * \param iDev          index of the device on \ref Devices
     * \return true 
     * \return false 
     */
    bool IsAddressClaimStarted(int iDev);

    /**********************************************************************//**
     * \brief Handles an IsoAddressClaim
     *
     * \param N2kMsg        Reference to a N2kMsg Object
     */
    void HandleISOAddressClaim(const tN2kMsg &N2kMsg);

    /**********************************************************************//**
     * \brief Handles if we get commanded to set a new address
     *
     * \param CommandedName   Device name that have been commanded to set 
     *                        new address
     * \param NewAddress      new address for the device  
     * \param iDev          index of the device on \ref Devices
     */
    void HandleCommandedAddress(uint64_t CommandedName, unsigned char NewAddress, int iDev);

     /**********************************************************************//**
     * \brief Handles if we get commanded to set a new address
     *
     * \param N2kMsg        Reference to a N2kMsg Object
     */
    void HandleCommandedAddress(const tN2kMsg &N2kMsg);

    /**********************************************************************//**
     * \brief Get the next free address for the device
     *
     * \param DeviceIndex     index of the device on \ref Devices
     * \param RestartAtEnd    Restart the search from the beginning if the 
     *                        search for the free source has reached 
     *                        \ref N2kNullCanBusAddress
     */
    void GetNextAddress(int DeviceIndex, bool RestartAtEnd=false);

    /**********************************************************************//**
     * \brief Checks if the source belongs to a device on \ref Devices 
     *
     * \param Source Source address
     * \return true 
     * \return false 
     */
    bool IsMySource(unsigned char Source);

    /**********************************************************************//**
     * \brief Finds a device on \ref Devices by its source address
     *
     * \param Source Source address of the device
     * \return int DeviceIndex, not found = -1
     */
    int FindSourceDeviceIndex(unsigned char Source);

    /**********************************************************************//**
     * \brief Get the Sequence Counter for the PGN
     *
     * \param PGN     PGN
     * \param iDev    index of the device on \ref Devices
     * \return int    Sequence Counter, non valid iDEV = 0
     */
    int GetSequenceCounter(unsigned long PGN, int iDev);

    /*********************************************************************//**
     * \brief Get the Fast Packet Tx PGN Count 
     *
     * \param iDev    index of the device on \ref Devices
     * \return size_t 
     */
    size_t GetFastPacketTxPGNCount(int iDev);

    /*********************************************************************//**
     * \brief Is message forwarding enabled
     * \sa  \ref ForwardMode, \ref N2kMode
     * Checks if forwarding i enabled and the node is not \ref N2km_SendOnly
     * 
     * \return true 
     * \return false 
     */
    bool ForwardEnabled() const { return ((ForwardMode&FwdModeBit_EnableForward)>0 && (N2kMode!=N2km_SendOnly)); }
    
    /*********************************************************************//**
     * \brief Is forwarding enabled for system messages
     * \sa  \ref ForwardMode
     * \return true 
     * \return false 
     */
    bool ForwardSystemMessages() const { return ((ForwardMode&FwdModeBit_SystemMessages)>0); }
    
    /*********************************************************************//**
     * \brief Is forwarding enabled for known messages only
     * \sa  \ref ForwardMode
     * \return true 
     * \return false 
     */
    bool ForwardOnlyKnownMessages() const { return ((ForwardMode&FwdModeBit_OnlyKnownMessages)>0); }

    /*********************************************************************//**
     * \brief Is forwarding enabled for own messages
     * \sa  \ref ForwardMode
     * \return true 
     * \return false 
     */
    bool ForwardOwnMessages() const { return ((ForwardMode&FwdModeBit_OwnMessages)>0); }

    /*********************************************************************//**
     * \brief Is handle only known messages enabled
     *
     * \return true 
     * \return false 
     */
    bool HandleOnlyKnownMessages() const { return ((ForwardMode&HandleModeBit_OnlyKnownMessages)>0); }

    /*********************************************************************//**
     * \brief Run all message handlers
     *
     * \param N2kMsg        Reference to a N2kMsg Object
     */
    void RunMessageHandlers(const tN2kMsg &N2kMsg);

    /*********************************************************************//**
     * \brief Should received message be handled depending on the destination
     *        of the received message
     *
     * \note  By default this message always returns true, because we handle
     *        messages to any destination.
     * 
     * \param Destination  Destination of the received message
     * \return true 
     * \return false 
     */
    bool HandleReceivedMessage(unsigned char Destination) {
      return (/* HandleMessagesToAnyDestination() */ true ||
              tNMEA2000::IsBroadcast(Destination) ||
              FindSourceDeviceIndex(Destination)>=0);
    }

    /*********************************************************************//**
     * \brief Returns if this node is active on the bus
     *
     * \return true ->  if ( \ref N2km_NodeOnly or \ref N2km_ListenAndNode)
     * \return false 
     */
    bool IsActiveNode() { return (N2kMode==N2km_NodeOnly || N2kMode==N2km_ListenAndNode); }

    /*********************************************************************//**
     * \brief Checks if the device index on \ref Devices is valid 
     *
     * \param iDev    index of the device on \ref Devices
     * \return true 
     * \return false 
     */
    bool IsValidDevice(int iDev) const { return (iDev>=0 && iDev<DeviceCount ); }

    /*********************************************************************//**
     * \brief Checks if the device is ready to send a message
     *
     * \return true 
     * \return false 
     */
    bool IsReadyToSend() const {
      return ( (OpenState==os_Open || dbMode!=dm_None) &&
               (N2kMode!=N2km_ListenOnly) &&
               (N2kMode!=N2km_SendOnly) &&
               (N2kMode!=N2km_ListenAndSend)
             );
    }


#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
    /*********************************************************************//**
     * \brief ISO Transport Protocol handlers for multi packet support
     * 
     * \param PGN           PGN 
     * \param Source        Source address 
     * \param Destination   Destination address
     * \param len           len of the data payload
     * \param buf           pointer to a byte buffer for the 
     * \param MsgIndex      MsgIndex for \ref N2kCANMsgBuf
     * 
     * \return true 
     * \return false 
     */
    bool TestHandleTPMessage(unsigned long PGN, unsigned char Source, unsigned char Destination,
                             unsigned char len, unsigned char *buf,
                             uint8_t &MsgIndex);

    /*********************************************************************//**
     * \brief   Send ISO Transport Protocol message BAM
     * 
     * This is used for Broadcast messages
     *
     * \param iDev    index of the device on \ref Devices
     * 
     * \return true 
     * \return false 
     */
    bool SendTPCM_BAM(int iDev);

    /*********************************************************************//**
     * \brief   Send ISO Transport Protocol message RTS
     *
     * \param iDev    index of the device on \ref Devices
     * 
     * \return true 
     * \return false 
     */
    bool SendTPCM_RTS(int iDev);

    /*********************************************************************//**
     * \brief   Send ISO Transport Protocol message CTS
     *
     * \param PGN                 PGN  
     * \param Destination         Destination address
     * \param iDev    index of the device on \ref Devices
     * \param nPackets            Number of packets
     * \param NextPacketNumber    Number of the next packet
     * 
     */
    void SendTPCM_CTS(unsigned long PGN, unsigned char Destination, int iDev, unsigned char nPackets, unsigned char NextPacketNumber);

    /**********************************************************************//**
     * \brief Send ISO Transport Protocol message End Acknowledge
     *
     * \param PGN                 PGN  
     * \param Destination         Destination address
     * \param iDev    index of the device on \ref Devices
     * \param nBytes              Number of bytes
     * \param nPackets            Number of packets
     */
    void SendTPCM_EndAck(unsigned long PGN, unsigned char Destination, int iDev, uint16_t nBytes, unsigned char nPackets);

    /*********************************************************************//**
     * \brief Send ISO Transport Protocol message Abort
     *
     * \param PGN               PGN  
     * \param Destination       Destination address
     * \param iDev    index of the device on \ref Devices
     * \param AbortCode         Abort Code 
     */
    void SendTPCM_Abort(unsigned long PGN, unsigned char Destination, int iDev, unsigned char AbortCode);

    /**********************************************************************//**
     * \brief Send ISO Transport Protocol data packet
     *
     * \note Caller should take care of not calling this after all has been 
     *        done. Use \ref HasAllTPDTSent for checking.
     * 
     * \param iDev    index of the device on \ref Devices
     * 
     * \return true   Message was send successful
     * \return false 
     */
    bool SendTPDT(int iDev);

    /*********************************************************************//**
     * \brief Check if all data bytes of the multi packet message has been send 
     *        successful
     *
     * \param iDev    index of the device on \ref Devices
     * 
     * \return true 
     * \return false 
     */
    bool HasAllTPDTSent(int iDev);

    /*********************************************************************//**
     * \brief Start sending an ISO-TP message
     *
     * \param msg     Reference to a N2kMsg Object
     * \param iDev    index of the device on \ref Devices
     * 
     * \return true 
     * \return false 
     */
    bool StartSendTPMessage(const tN2kMsg& msg, int iDev);

    /*********************************************************************//**
     * \brief Ends sending of ISO-TP message
     *
     * \param iDev    index of the device on \ref Devices
     */
    void EndSendTPMessage(int iDev);

    /*********************************************************************//**
     * \brief Send pending ISO-TP Messages
     *
     * \param iDev    index of the device on \ref Devices
     */
    void SendPendingTPMessage(int iDev);
#endif
#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)
    /*********************************************************************//**
     * \brief Copy Configuration Information to local memory
     *
     * \sa  \ref SetConfigurationInformation, \ref SetInstallationDescription1
     *      and \ref SetInstallationDescription2
     */
    void CopyProgmemConfigurationInformationToLocal();

    /*********************************************************************//**
     * \brief Flag the Installation description has changed
     */
    bool InstallationDescriptionChanged;
#endif
public:
    /*********************************************************************//**
     * \brief Construct a new NMEA2000 object
     * 
     * Initialize all the attributes of the class
     */
    tNMEA2000();

    /*********************************************************************//**
     * \brief Set the Device Counter
     *
     * With this function you can enable multi device support. As default 
     * there is only one device.
     * 
     * \note To enable multi device support, you need to call this before 
     * any other tNMEA2000 class function. 
     * 
     * \sa  \ref descMultiDeviceSupport
     * 
     * \param _DeviceCount  Maximal number of devices that can be hold in 
     *                      \ref Devices
     */
    void SetDeviceCount(const uint8_t _DeviceCount);

    // 

    /*********************************************************************//**
     * \brief Set the maximum buffersize of \ref N2kCANMsgBuf
     *
     * With this function you can set size of buffer, where system stores 
     * incoming messages. The default size is 5 messages. Some messages are 
     * just single frame messages and they will be read in and handled 
     * immediately on call to ParseMessages. For multi framed fast packet 
     * messages there is no guarantee that all frames will arrive in order.
     * So these messages will be buffered and saved until all frames has
     * been received.
     * If it is not critical to handle all fast packet messages like with 
     * N2km_NodeOnly, you can set buffer size smaller like 3 or 2 by calling 
     * this before \ref tNMEA2000::Open().
     * 
     * \param _MaxN2kCANMsgs  Maximal number of CAN messages that can be 
     *                        stored in \ref N2kCANMsgBuf
     */
    void SetN2kCANMsgBufSize(const uint8_t _MaxN2kCANMsgs) { if (N2kCANMsgBuf==0) { MaxN2kCANMsgs=_MaxN2kCANMsgs; }; }

    /*********************************************************************//**
     * \brief Set the maximum buffersize of \ref CANSendFrameBuf
     * 
     * When sending long messages like ProductInformation or GNSS data, 
     * there may not be enough buffers for successfully send data. This 
     * depends of your hw and device source. Device source has effect due 
     * to priority of getting sending slot. If your data is critical, use 
     * buffer size, which is large enough (default 40 frames).
     * So e.g. Product information takes totally 134 bytes. This needs 20 
     * frames. If you also send GNSS 47 bytes=7 frames.
     * If you want to be sure that both will be sent on any situation, 
     * you need at least 27 frame buffer size.
     * If you use this function, call it once before \ref tNMEA2000::Open() and 
     * before any device related function like SetProductInformation.
     *
     * \param _MaxCANSendFrames Maximal number of CAN messages that can be 
     *                          stored in \ref CANSendFrameBuf
     */
    virtual void SetN2kCANSendFrameBufSize(const uint16_t _MaxCANSendFrames) { if ( !IsInitialized() ) { MaxCANSendFrames=_MaxCANSendFrames; }; }

    /*********************************************************************//**
     * \brief Set the maximum buffersize for received CAN messages
     *
     * Some CAN drivers allows interrupted receive frame buffering. You can 
     * set receive buffer size with this function. If you use this function, 
     * call it once before \ref tNMEA2000::Open();
     * 
     * \param _MaxCANReceiveFrames {type} 
     */
    virtual void SetN2kCANReceiveFrameBufSize(const uint16_t _MaxCANReceiveFrames) { if ( !IsInitialized() ) MaxCANReceiveFrames=_MaxCANReceiveFrames; }

    

    /*********************************************************************//**
     * \brief Set the Product Information of this device
     *
     * Define your product information. Defaults will be set on initialization.
     * For keeping defaults use 0xffff/0xff for int/char values and nul ptr
     * for pointers. LoadEquivalency is multiplication of 50 mA, what your 
     * device will take power from N2k-bus. E.g. for Arduino only it can 
     * be 1 (=50mA). If your device does not take power from bus, set this 
     * to 0.
     * 
     * \note Serial code has length of 32 so just long enough to carry GUID.
     * 
     * \param _ModelSerialCode      Default="00000001". Max 32 chars. 
     *                              Manufacturer's Model serial code
     * \param _ProductCode          Default=666. Manufacturer's product code
     * \param _ModelID              Default="Arduino N2k->PC". Max 33 chars. 
     *                              Manufacturer's  Model ID
     * \param _SwCode               Default="1.0.0.0". Max 40 chars. 
     *                              Manufacturer's software version code
     * \param _ModelVersion         Default="1.0.0". Max 24 chars. 
     *                              Manufacturer's Model version
     * \param _LoadEquivalency      Default=1. x * 50 mA
     * \param _N2kVersion           Default=2101
     * \param _CertificationLevel   Default=1
     * \param iDev    index of the device on \ref Devices
     */
    void SetProductInformation(const char *_ModelSerialCode, 
                               unsigned short _ProductCode=0xffff,  
                               const char *_ModelID=0, 
                               const char *_SwCode=0, 
                               const char *_ModelVersion=0, 
                               unsigned char _LoadEquivalency=0xff,  
                               unsigned short _N2kVersion=0xffff, 
                               unsigned char _CertificationLevel=0xff, 
                               int iDev=0
                               );
    
    /*********************************************************************//**
     * \brief Set the Product Information of this device
     *
     * Call this if you want to save RAM and you have defined \ref 
     * tProductInformation to PROGMEM as in example BatteryMonitor.ino
     * 
     * \note I have not yet found a way to test is pointer in PROGMEM or not,
     * so this does not work, if you define tProductInformation to RAM.
     * 
     * \param _ProductInformation   Productinformation, see \ref 
     *                              tProductInformation
     * \param iDev    index of the device on \ref Devices
     */
    void SetProductInformation(const tProductInformation *_ProductInformation, int iDev=0);

    /*********************************************************************//**
     * \brief Set the Configuration Information of this device
     *
     * Configuration information is just some extra information 
     * about device installation and manufacturer. Some MFD shows it, some 
     * does not. NMEA Reader can show configuration information.
     * InstallationDescription1 and InstallationDescription2 can be 
     * changed as default during runtime by NMEA 2000 group function 
     * commands. That can be done e.g. with NMEA Reader.    
     * 
     * \note You can disable configuration information by calling 
     * SetProgmemConfigurationInformation(0);
     * 
     * \param ManufacturerInformation     Buffer for Manufacturer information
     * \param InstallationDescription1    Buffer for Installation 
     *                                    Description 1
     * \param InstallationDescription2    Buffer for Installation 
     *                                    Description 2
     */
    void SetConfigurationInformation(const char *ManufacturerInformation,
                                     const char *InstallationDescription1=0,
                                     const char *InstallationDescription2=0);

    /*********************************************************************//**
     * \brief Set the Progmem Configuration Information of this device
     *
     * With this function you can set configuration information, which 
     * will be saved on device program memory.
     * See example BatteryMonitor.ino.  
     * 
     * As default system has build in configuration information on progmem. 
     * If you do not want to have configuration information at all, 
     * you can disable it by calling SetConfigurationInformation(0);
     * 
     * \param ManufacturerInformation     Buffer for Manufacturer information
     * \param InstallationDescription1    Buffer for Installation 
     *                                    Description 1
     * \param InstallationDescription2    Buffer for Installation 
     *                                    Description 2
     */
    void SetProgmemConfigurationInformation(const char *ManufacturerInformation,
                                     const char *InstallationDescription1=0,
                                     const char *InstallationDescription2=0);

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)

    /**********************************************************************//**
     * \brief Check if this message is a Transmit message of this device 
     *
     * \param PGN     PGN to be checked
     * \param iDev    index of the device on \ref Devices
     * \return true 
     * \return false 
     */
    bool IsTxPGN(unsigned long PGN, int iDev=0);

    /**********************************************************************//**
     * \brief Get the Product Information of the device
     *
     * \param iDev        index of the device on \ref Devices
     * \param IsProgMem   Program memory has been used for the data
     * \return const tNMEA2000::tProductInformation* 
     */
    const tNMEA2000::tProductInformation * GetProductInformation(int iDev, bool &IsProgMem) const;

    /**********************************************************************//**
     * \brief Get the N2k standard version of the device
     *
     * \param iDev      index of the device on \ref Devices
     * \return unsigned short 
     */
    unsigned short GetN2kVersion(int iDev=0) const;

    /**********************************************************************//**
     * \brief Get the Product Code of the device
     *
     * \param iDev      index of the device on \ref Devices
     * \return unsigned short 
     */
    unsigned short GetProductCode(int iDev=0) const;

    /**********************************************************************//**
     * \brief Get the ModelID of the device
     *
     * \param buf       Buffer to hold the information
     * \param max_len   Maximum size of the buffer
     * \param iDev      index of the device on \ref Devices
     */
    void GetModelID(char *buf, size_t max_len, int iDev=0) const;

    /**********************************************************************//**
     * \brief Get the Sw Code of the device
     *
     * \param buf       Buffer to hold the information
     * \param max_len   Maximum size of the buffer
     * \param iDev      index of the device on \ref Devices
     */
    void GetSwCode(char *buf, size_t max_len, int iDev=0) const;

    /**********************************************************************//**
     * \brief Get the Model Version of the device
     *
     * \param buf       Buffer to hold the information
     * \param max_len   Maximum size of the buffer
     * \param iDev      index of the device on \ref Devices
     */
    void GetModelVersion(char *buf, size_t max_len, int iDev=0) const;

    /**********************************************************************//**
     * \brief Get the Model Serial of the device
     *
     * \param buf       Buffer to hold the information
     * \param max_len   Maximum size of the buffer
     * \param iDev      index of the device on \ref Devices
     */
    void GetModelSerialCode(char *buf, size_t max_len, int iDev=0) const;

    /**********************************************************************//**
     * \brief Get the Certification Level of the device
     *
     * \param iDev      index of the device on \ref Devices
     * \return unsigned char 
     */
    unsigned char GetCertificationLevel(int iDev=0) const;

    /*********************************************************************//**
     * \brief Get the Load Equivalency of this device
     *
     * A Load Equivalence Number express the amount of current that
     * is drawn from an NMEA 2000 network. 1 equals to 50mA. If 
     * a device draws 151mA of current from the network, then its 
     * LEN is 4
     * 
     * \param iDev      index of the device on \ref Devices
     * \return unsigned char 
     */
    unsigned char GetLoadEquivalency(int iDev=0) const;
    
    /*********************************************************************//**
     * \brief Set the Installation Description 1 of this device
     *
     * This is automatically used by class. You only need to use this, if you 
     * want to write your own behavior.
     * 
     * \param InstallationDescription1 Description
     */
    void SetInstallationDescription1(const char *InstallationDescription1);
    
    /*********************************************************************//**
     * \brief Set the Installation Description 2 of this device
     *
     * This is automatically used by class. You only need to use this, if you 
     * want to write your own behavior.
     * 
     * \param InstallationDescription2 Description
     */
    void SetInstallationDescription2(const char *InstallationDescription2);
    
    /*********************************************************************//**
     * \brief Get the Install Description 1 of this device
     * 
     * \param buf       Buffer in memory for the description
     * \param max_len   Max size of the buffer
     * 
     */
    void GetInstallationDescription1(char *buf, size_t max_len);
    
    /*********************************************************************//**
     * \brief Get the Install Description 2 of this device
     * 
     * \param buf       Buffer in memory for the description
     * \param max_len   Max size of the buffer
     * 
     */
    void GetInstallationDescription2(char *buf, size_t max_len);
    
    /*********************************************************************//**
     * \brief Get the Manufacturer Information of this device
     * 
     * \param buf       Buffer in memory for the description
     * \param max_len   Max size of the buffer
     * 
     */
    void GetManufacturerInformation(char *buf, size_t max_len);
    
    /*********************************************************************//**
     * \brief Check if this device has changed its Install Description
     *
     * \return true 
     * \return false 
     */
    bool ReadResetInstallationDescriptionChanged();
#endif

    /*********************************************************************//**
     * \brief Set the list of known Single Frame Messages
     * 
     * Call these if you wish to override the default message packets 
     * supported.  Pointers must be in PROGMEM
     * 
     * \code {.cpp}
     * const unsigned SingleFrameMessages[] PROGMEM={......L,0};
     * \endcode
     * 
     * As a default library has a list of known messages. With this 
     * function user can override default list of single frame messages.
     * 
     * \param _SingleFrameMessages  Buffer holding single frame messages 
     * 
     */
    void SetSingleFrameMessages(const unsigned long *_SingleFrameMessages);

    /*********************************************************************//**
     * \brief Set the list of known Fast Packet Messages
     * 
     * Call these if you wish to override the default message packets 
     * supported.  Pointers must be in PROGMEM
     * 
     * As a default library has a list of known messages. With this 
     * function user can override default list of fast packet messages. 
     * See also \ref tNMEA2000::ExtendFastPacketMessages.
     * 
     * \note If an incoming fast packet message is not known, it will be 
     * treated as single frame message. So if you want to handle unknown 
     * fast packet message, you need to duplicate frame collection logic 
     * from library to your code. So it is easier to have fast packet 
     * messages listed on library, if you want to handle them.

     * \param _FastPacketMessages  Buffer holding fast packet messages
     * 
     */
    void SetFastPacketMessages (const unsigned long *_FastPacketMessages);
    
    /*********************************************************************//**
     * \brief Set the list of known Extended Single Frame Messages
     * 
     * Call these if you wish to add own list of supported message 
     * packets.  Pointers must be in PROGMEM
     * 
     * As a default library has a list of known messages. With this 
     * function user can add own list of known single frame messages.
     * 
     * \note Currently subsequent calls will override previously set list.
     * 
     * \param _SingleFrameMessages  Buffer holding single frame messages
     * 
     * \sa see \ref SetSingleFrameMessages
     */
    void ExtendSingleFrameMessages(const unsigned long *_SingleFrameMessages);

    /*********************************************************************//**
     * \brief Set the list of known Extended Fast Packet Messages
     * 
     * Call these if you wish to override the default message packets 
     * supported.  Pointers must be in PROGMEM
     * 
     * As a default library has a list of known messages. With this 
     * function user can override default list of fast packet messages. 
     * See also \ref tNMEA2000::ExtendFastPacketMessages.
     * 
     * \note If an incoming fast packet message is not known, it will be 
     * treated as single frame message. So if you want to handle unknown 
     * fast packet message, you need to duplicate frame collection logic 
     * from library to your code. So it is easier to have fast packet 
     * messages listed on library, if you want to handle them.
     * 
     * \note Currently subsequent calls will override previously set list.
     * 
     * \param _FastPacketMessages  Buffer holding extended fast packet messages
     * 
     */
    void ExtendFastPacketMessages (const unsigned long *_FastPacketMessages);

    /*********************************************************************//**
     * \brief Extend the list of Transmitted Messages
     * 
     * Define information about PGNs, what your system can handle.  
     * Pointers must be in PROGMEM
     * 
     * \code {.cpp}
     * const unsigned TransmitMessages[] PROGMEM={130310L,130311L,130312L,0};
     * 
     * NMEA2000.ExtendTransmitMessages(TransmitMessages);
     * \endcode
     * 
     * System should respond to PGN 126464 request with messages the 
     * system transmits and receives. The library will automatically respond 
     * with system the messages it handles internally. With this method you 
     * can add messages, which your own code sends
     * 
     * \note that this is valid only for device modes 
     *       \ref tNMEA2000::N2km_NodeOnly and \ref 
     *       tNMEA2000::N2km_ListenAndNode.
     * 
     * \param _SingleFrameMessages  Buffer holding user code defined transmit messages 
     * \param iDev    index of the device on \ref Devices
     */
    void ExtendTransmitMessages(const unsigned long *_SingleFrameMessages, int iDev=0);

    /*********************************************************************//**
     * \brief Extend the list of Received Messages
     * 
     * Define information about PGNs, what your system can handle.  
     * Pointers must be in PROGMEM
     * 
     * \code {.cpp}
     * const unsigned ReceivedMessages[] PROGMEM={130310L,130311L,130312L,0};
     * 
     * NMEA2000.ExtendReceiveMessages(ReceivedMessages);
     * \endcode
     * 
     * System should respond to PGN 126464 request with messages the 
     * system transmits and receives. The library will automatically respond 
     * with system the messages it uses. With this method you can add 
     * messages, which your own code sends
     * 
     * \note that this is valid only for device modes 
     *       \ref tNMEA2000::N2km_NodeOnly and \ref 
     *       tNMEA2000::N2km_ListenAndNode.
     * 
     * \param _FastPacketMessages  Buffer holding user code defined received messages 
     * \param iDev    index of the device on \ref Devices
     */
    void ExtendReceiveMessages(const unsigned long *_FastPacketMessages, int iDev=0);

    
    /*********************************************************************//**
     * \brief Set the Device Information 
     *
     * If you are using device modes tNMEA2000::N2km_NodeOnly or 
     * tNMEA2000::N2km_ListenAndNode, it is critical that you set this 
     * information.
     * 
     * Device information will be used to choose right address for your 
     * device (also called node) on the bus. Each device must have an own 
     * address. Library will do this automatically, so it is enough that 
     * you call this function on setup to define your device.
     * 
     * For keeping defaults use 0xffff/0xff for int/char values and 
     * nul ptr for pointers.
     * 
     * \note You should set information so that it is unique over the 
     * world! Well if you are making device only for your own yacht N2k 
     * bus, it is enough to be unique there. So e.g. if you have two 
     * temperature monitors made by this library, you have to set at 
     * least first parameter UniqueNumber different for both of them.  
     * 
     * I just decided to use number below for ManufacturerCode as Open 
     * Source devices - this is not any number given by NMEA.
     * 
     * \param _UniqueNumber     Default=1. 21 bit resolution, max 2097151. 
     *                          Each device from same manufacturer should 
     *                          have unique number.
     * \param _DeviceFunction   Default=130, PC Gateway. See codes on 
     *  https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
     * \param _DeviceClass      Default=25, Inter/Intranetwork Device. 
     *                          See codes on 
     * https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
     * \param _ManufacturerCode Default=2046. Maximum 2046. See the 
     *                          list of codes on 
     * https://web.archive.org/web/20190529161431/http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
     * \param _IndustryGroup    Default=4, Marine.
     * \param iDev    index of the device on \ref Devices
     */
    void SetDeviceInformation(unsigned long _UniqueNumber, 
                              unsigned char _DeviceFunction=0xff, 
                              unsigned char _DeviceClass=0xff, 
                              uint16_t _ManufacturerCode=0xffff,  
                              unsigned char _IndustryGroup=4,  
                              int iDev=0
                              );

    /*********************************************************************//**
     * \brief Set the Device Information Instances
     *
     * With this function you can set device instance lower, device 
     * instance upper and system instance values.
     * 
     * \param _DeviceInstanceLower  0xff means no change
     * \param _DeviceInstanceUpper  
     * \param _SystemInstance       
     * \param iDev    index of the device on \ref Devices
     */
    void SetDeviceInformationInstances(
                              uint8_t _DeviceInstanceLower=0xff, 
                              uint8_t _DeviceInstanceUpper=0xff,
                              uint8_t _SystemInstance=0xff,
                              int iDev=0
                              );

    /*********************************************************************//**
     * \brief Get the Device Information 
     * 
     * With this function you can read current device information. Normally 
     * device information contains what you have set during initializing 
     * with \ref SetDeviceInformation and \ref SetDeviceInformationInstances
     * functions.
     * 
     * \note Device information instances can be changed by the NMEA 2000 
     * group function by e.g. using system configuration device. So you 
     * should time to time check if they have changed and save changed 
     * data to e.g. EEPROM for use on startup.  
     * 
     * See \ref tNMEA2000::ReadResetDeviceInformationChanged
     * 
     * \param iDev    index of the device on \ref Devices
     * 
     * \return const tDeviceInformation 
     */
    const tDeviceInformation GetDeviceInformation(int iDev=0) { if (iDev<0 || iDev>=DeviceCount) return tDeviceInformation(); return Devices[iDev].DeviceInformation; }

    /*********************************************************************//**
     * \brief Send an IsoAddressClaim message
     *
     * This is automatically used by class. You only need to use this, if 
     * you want to write your own behavior for address claiming.
     * 
     * \param Destination     Destination address for the message
     * \param DeviceIndex     index of the device on \ref Devices
     * \param FromNow         optional time delay from now in ms
     */
    void SendIsoAddressClaim(unsigned char Destination=0xff, int DeviceIndex=0, unsigned long FromNow=0);

#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
    /*********************************************************************//**
     * \brief Send a Product Information message
     *
     * This is automatically used by class. You only need to use this, if 
     * you want to write your own behavior for providing product information.
     * 
     * \param Destination   Destination address
     * \param DeviceIndex   index of the device on \ref Devices
     * \param UseTP         use multi packet message
     * \return true         -> Success
     * \return false 
     */
    bool SendProductInformation(unsigned char Destination, int DeviceIndex, bool UseTP);

    /*********************************************************************//**
     * \brief Send a Config Information message
     *
     * This is automatically used by class. You only need to use this, if 
     * you want to write your own behavior for providing config information.
     * 
     * \param Destination   Destination address
     * \param DeviceIndex   index of the device on \ref Devices
     * \param UseTP         use multi packet message
     * \return true         -> Success
     * \return false 
     */
    bool SendConfigurationInformation(unsigned char Destination, int DeviceIndex, bool UseTP);

    /*********************************************************************//**
     * \brief Send a list with all supported Transmit messages
     * 
     * This function sends a PGN 126464 message consisting of all messages 
     * supported by this device for transmission. 
     *
     * \param Destination   Destination address
     * \param DeviceIndex   index of the device on \ref Devices
     * \param UseTP         use multi packet message, default = false
     */
    void SendTxPGNList(unsigned char Destination, int DeviceIndex, bool UseTP=false);

    /*********************************************************************//**
     * \brief Send a list with all supported Receive messages
     * 
     * This function sends a PGN 126464 message consisting of all messages 
     * supported by this device for reception. 
     *
     * \param Destination   Destination address
     * \param DeviceIndex   index of the device on \ref Devices
     * \param UseTP         use multi packet message, default = false
     */
    void SendRxPGNList(unsigned char Destination, int DeviceIndex, bool UseTP=false);
#else
    void SendTxPGNList(unsigned char Destination, int DeviceIndex);
    void SendRxPGNList(unsigned char Destination, int DeviceIndex);
#endif

    /*********************************************************************//**
     * \brief Send a Product Information message
     *
     * This is automatically used by class. You only need to use this, if 
     * you want to write your own behavior for providing product information.
     * 
     * \param DeviceIndex   index of the device on \ref Devices
     * \return true  ->  Success
     * \return false 
     */
    bool SendProductInformation(int DeviceIndex=0);

    /*********************************************************************//**
     * \brief Send a Config Information message
     *
     * This is automatically used by class. You only need to use this, if 
     * you want to write your own behavior for providing config information.
     * 
     * \param DeviceIndex   index of the device on \ref Devices
     * \return true         -> Success
     * \return false 
     */
    bool SendConfigurationInformation(int DeviceIndex=0);

#if !defined(N2K_NO_HEARTBEAT_SUPPORT)
    

    /*********************************************************************//**
     * \brief Set the Heartbeat Interval and Offset for a device
     * 
     * According to document [NMEA Heartbeat Corrigendum] 
     * (https://web.archive.org/web/20170609023206/https://www.nmea.org/Assets/20140102%20nmea-2000-126993%20heartbeat%20pgn%20corrigendum.pdf)
     * all NMEA devices shall transmit heartbeat PGN 126993. With this 
     * function you can set transmission interval in ms (range 1000-655320ms
     * , default 60000). Set <1000 to disable it.
     * You can temporally change interval by setting SetAsDefault parameter 
     * to false. Then you can restore default interval with interval 
     * parameter value 0xfffffffe
     *
     * Function allows to set interval over 60 s or 0 to disable sending fr test purposes.
     *
     * \param interval      Heartbeat Interval in ms
     * \param offset  		Heartbeat Offset in ms
     * \param iDev          index of the device on \ref Devices
     */
    void SetHeartbeatIntervalAndOffset(uint32_t interval, uint32_t offset=0, int iDev=-1);
    
    /*********************************************************************//**
     * \brief Get the Heartbeat Interval of a device
     *
     * Heartbeat interval may be changed by e.g. MFD by group function. I 
     * have not yet found should changed value be saved for next 
     * startup or not.
     * 
     * \param iDev          index of the device on \ref Devices
     * \return uint_32  -> Device heartbeat interval in ms
     */
    uint32_t GetHeartbeatInterval(int iDev=0) { if (iDev<0 || iDev>=DeviceCount) return 60000; return Devices[iDev].HeartbeatScheduler.GetPeriod(); }
    /*********************************************************************//**
     * \brief Get the Heartbeat Offset of a device
     *
     * Heartbeat Offset may be changed by e.g. MFD by group function. I 
     * have not yet found should changed value be saved for next 
     * startup or not.
     * 
     * \param iDev          index of the device on \ref Devices
     * \return uint_32  -> Device heartbeat Offset in ms
     */
    uint32_t GetHeartbeatOffset(int iDev=0) { if (iDev<0 || iDev>=DeviceCount) return 0; return Devices[iDev].HeartbeatScheduler.GetOffset(); }
	
    /*********************************************************************//**
     * \brief Send heartbeat for specific device.
     * 
     * \param iDev          index of the device on \ref Devices
     */
    void SendHeartbeat(int iDev);
    
    /*********************************************************************//**
     * \brief Send Heartbeat for all devices
     *
     * Library will automatically send heartbeat, if interval is >0. You 
     * can also manually send it any time or force sent, if interval=0;
     *
     * \param force True will send Heartbeat immediately, default = false 
     */
    void SendHeartbeat(bool force=false);

    // deprecated! SetAsDefault has no effect. Use function SetHeartbeatIntervalAndOffset.
    void SetHeartbeatInterval(unsigned long interval, bool SetAsDefault=true, int iDev=-1) __attribute__ ((deprecated));
#endif

    /*********************************************************************//**
     * \brief Set the Mode object
     *
     * With SetMode you can define how your node acts on N2k bus. 
     * See \ref tNMEA2000::Devices modes.
     * 
     * With this function you can also set default address for your device. 
     * It is mandatory that once your device has been connected to the bus, 
     * it tries always use last used address. Due to address claiming, your 
     * device may change its address, when you add new devices to the bus.
     * So you should save last used address to the e.g. EEPROM and on startup 
     * read it there and use it as parameter for SetMode. You can check if 
     * your address you set originally by SetMode has changed by using 
     * function \ref tNMEA2000::SetN2kSource and you can read current 
     * address by function \ref tNMEA2000::GetN2kSource. If you know your
     * system, define source something other address you allready have on
     * your bus.
     *
     * \note Other than N2km_ListenOnly modes will automatically start
     * initialization and address claim procedure.
     
     * \param _N2kMode    Mode for this node, see \ref tN2kMode
     * \param _N2kSource  Source address for this node
     */
    void SetMode(tN2kMode _N2kMode, uint8_t _N2kSource=15);

    /*********************************************************************//**
     * \brief Set the Forward Streaming Type
     *
     * With this function user can set how messages will be forwarded to 
     * the stream. Possible values are:
     * - tNMEA2000::fwdt_Actisense (default) forwards messages is Actisense 
     *    format. Some navigation softwares can read this format.
     * - tNMEA2000::fwdt_Text  forwards messages to output port in clear 
     *    text. I see this useful only for testing with normal serial 
     *    monitors.
     * 
     * \param fwdType Format type see \ref tForwardType, 
     *                default = fwdt_Actisense
     */
    void SetForwardType(tForwardType fwdType) { ForwardType=fwdType; }

    /*********************************************************************//**
     * \brief Set the Forward Stream object
     *
     * As default, forward stream has been set to null. For e.g. Arduino 
     * Due you can set it to SerialUSB, so you can use Serial for other 
     * things. You can of coarse use any stream available on your device.  
     * See example ActisenseListenerSender.ino.
     * 
     * \param _stream Stream to be used for message forwarding
     */
    void SetForwardStream(N2kStream* _stream) { ForwardStream=_stream; }

    /*********************************************************************//**
     * \brief Open the CAN device
     *
     * You can call this on Setup(). It will be called anyway automatically 
     * by first call of ParseMessages(). When system is finally opened,
     * OnOpen callback will be executed. See \ref SetOnOpen
     * 
     * \note After Open() you should start loop ParseMessages() without delays.
     * 
     * \return true 
     * \return false 
     */
    bool Open();

    /*********************************************************************//**
     * \brief Restart the device
     * 
     * This is preliminary function for e.g. battery powered or devices, 
     * which may go to sleep or of the bus in any way. Function is under 
     * development.
     * 
     * \note At the moment this functions just restarts the IsoAddressClaim
     */
    void Restart();

    /*********************************************************************//**
     * \brief Send a N2k message to the bus
     *
     * Generate N2k message e.g. by using N2kMessages.h and simply send 
     * it to the bus.
     * 
     * When you want to send some message to the N2k bus, you call this. 
     * Before calling you have to prepare tN2kMsg type of message e.g. by 
     * using some function in N2kMessages.  
     * 
     * \note As default tNMEA2000 object is as default in 
     * tNMEA2000::N2km_ListenOnly mode. So if you want to send messages, 
     * you have to set right mode in Setup().
     * 
     * The function returns true, if the message was sent successfully, 
     * otherwise it return false. SendMsg may fail, if there is not room 
     * for message frames on sending buffer or device is not open.  
     * SendMsg does not always send message immediately. If lower level 
     * sending function fails, SendMsg will buffer message frames and 
     * try to send them on next call to SendMsg or ParseMessages. So 
     * to have reliable sending, you need a sending buffer, which is 
     * large enough.  
     * 
     * See example TemperatureMonitor.ino.
     * 
     * \param N2kMsg          Reference to a N2kMsg Object
     * \param DeviceIndex     index of the device on \ref Devices
     * \return true         -> Success
     * \return false 
     */
    bool SendMsg(const tN2kMsg &N2kMsg, int DeviceIndex=0);

    /*********************************************************************//**
     * \brief Parse all incoming Messages
     *
     * You have to call this periodically on loop()to handle N2k messages,
     * otherwise tNMEA2000 object will not work at all.
     * 
     * \note It is not good practice to have any delay() on your loop(), 
     * since then also handling of this will be delayed.
     * 
     * \note Even if you only send e.g. temperature to the bus, you should 
     * call this so the node will automatically inform about itself to 
     * others. Take care that your loop to call ParseMessages() does not
     * have long delays. 
     * 
     * See example TemperatureMonitor.ino.
     */
    void ParseMessages();
    
    /*********************************************************************//**
     * \brief Set OnOpen callback function
     *
     * This will be called, when communication really opens
     * and starts initial address claiming. You can use this to init your message sending
     * to syncronize them with e.g., heartbeat
     */
    void SetOnOpen(void (*_OnOpen)());

    /*********************************************************************//**
     * \brief Set the message handler for incoming N2kMessages
     *
     * If you want to do something with messages read from N2k bus, easiest 
     * is to set message handler, which will be then called by ParseMessages, 
     * if there are new messages. This is the case e.g. if you have LCD 
     * display on your Arduino and you want to show some fluid level on it.  
     * 
     * See example DataDisplay.ino or DataDisplay2.ino
     * 
     * \param _MsgHandler Old style - callback function pointer
     */
    void SetMsgHandler(void (*_MsgHandler)(const tN2kMsg &N2kMsg));

    /*********************************************************************//**
     * \brief Attach a  message handler for incoming N2kMessages
     * 
     * \ref SetMsgHandler allows you to define only one handler to your 
     * system. If you like to do it by using classes, I prefer to use 
     * AttachMsgHandler. In this way you can e.g. define own class for
     * each PGN and attach/detach them within your program. 
     * 
     * Example NMEA2000ToNMEA0183 uses AttachMsgHandler. Due to logic it 
     * still has single class and so handles all PGNs.
     * 
     * \param _MsgHandler  Message handler
     */
    void AttachMsgHandler(tMsgHandler *_MsgHandler);

    /*********************************************************************//**
     * \brief Detach a  message handler for incoming N2kMessages
     * 
     * With DetachMsgHandler you can remove your handler from the handler 
     * stack. This is useful, if you do not want to handle some messages 
     * anymore.
     * 
     * \sa \ref AttachMsgHandler
     * 
     * \param _MsgHandler Message handler
     */
    void DetachMsgHandler(tMsgHandler *_MsgHandler);

    /*********************************************************************//**
     * \brief Set the message handler for incoming ISO Requests
     *
     * Devices on N2k bus may request from your device if it can handle 
     * requested PGN. If you want to respond for ISO request, you should 
     * set this handler. The handler will be called by ParseMessages, if 
     * there is ISO request.  
     * 
     * \note When you send request message with SendMsg and it fails, it 
     * is your responsibility to take care of sending response again later. 
     * If your sending buffer is large enough, it is very uncommon 
     * that SendMsg fails.
     * 
     * \param ISORequestHandler Message handler
     */
    void SetISORqstHandler(bool(*ISORequestHandler)(unsigned long RequestedPGN, unsigned char Requester, int DeviceIndex));

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)
    
    /**********************************************************************//**
     * \brief Remove a message handler for incoming Group Function messages
     *
     * \param pGroupFunctionHandler Message handler for group functions, see
     *                              \ref tN2kGroupFunctionHandler
     */
    void RemoveGroupFunctionHandler(tN2kGroupFunctionHandler *pGroupFunctionHandler);

    /**********************************************************************//**
     * \brief Add a message handler for incoming Group Function messages
     *
     * \param pGroupFunctionHandler Message handler for group functions, see
     *                              \ref tN2kGroupFunctionHandler
     */
    void AddGroupFunctionHandler(tN2kGroupFunctionHandler *pGroupFunctionHandler);
#endif

    /*********************************************************************//**
     * \brief Read address for current device.
     *
     * With this function you can get you device current address on the 
     * N2k bus.  
     * 
     * See \ref tNMEA2000::SetMode and \ref tNMEA2000::SetN2kSource
     * 
     * \note Multidevice support is under construction.
     * 
     * \param DeviceIndex   index of the device on \ref Devices
     * \return unsigned char -> Address of the device 
     */
    unsigned char GetN2kSource(int DeviceIndex=0) const { if (DeviceIndex>=0 && DeviceIndex<DeviceCount) return Devices[DeviceIndex].N2kSource; return Devices[0].N2kSource; }
    
    /*********************************************************************//**
     * \brief Set source for the given device
     *
     * With this function you can set you device current address on the 
     * N2k bus. This is meant to be use for multi device on basic 
     * configuration to restore source address changed by address claiming.  
     * 
     * This function has to be called after \ref SetMode() and before 
     * \ref Open()
     * 
     * See \ref tNMEA2000::SetMode and \ref tNMEA2000::SetN2kSource
     * 
     * \param _iAddr  Address of the device
     * \param _iDev   index of the device on \ref Devices
     */
    void SetN2kSource(unsigned char _iAddr, int _iDev=0);

    /*********************************************************************//**
     * \brief Check if this device has changed its address
     * 
     * With this function you can check has your device address you initiated 
     * with SetMode been changed after last call. 
     * 
     * \note For certified NMEA 2000 devices it is mandatory save changed 
     * address to e.g. EEPROM, for use in next startup.  
     * 
     * When you call this, \ref AddressChanged will be reset. Anyway, if system 
     * for some reason needs to change its address again, \ref AddressChanged 
     * will be set. So you can e.g. in every 10 min check has address 
     * changed and if it has, save it.
     * 
     * See \ref tNMEA2000::SetMode and \ref tNMEA2000::SetN2kSource
     * 
     * \return true 
     * \return false 
     */
    bool ReadResetAddressChanged();

    /*********************************************************************//**
     * \brief Check if this device has changed its DeviceInstances or 
     * SystemInstance
     *
     * With this function you can check has your device device instances or 
     * system instances changed. 
     * 
     * \note For certified NMEA 2000 devices it is mandatory save changed 
     * info to e.g. EEPROM, for initialize them in next startup.  
     * 
     * See \ref tNMEA2000::SetDeviceInformationInstances and 
     *      \ref tNMEA2000::GetDeviceInformation
     * 
     * \return true 
     * \return false 
     */
    bool ReadResetDeviceInformationChanged();

    /*********************************************************************//**
     * \brief Enable message forwarding to stream
     * 
     * Set true as default. With this you can control if bus messages 
     * will be forwarded to forward stream.  
     * 
     * see \sa Message forwarding.
     * 
     * \param v   Enable, default = true
     */
    void EnableForward(bool v=true) {
        if (v) { ForwardMode |= FwdModeBit_EnableForward;  } else { ForwardMode &= ~FwdModeBit_EnableForward; }
    }

    /*********************************************************************//**
     * \brief Enable System Messages for forwarding 
     *
     * Set true as default. With this you can control if system messages 
     * like address claiming, device information will be forwarded to 
     * forward stream.  If you set this false, system messages will not 
     * be forwarded to the stream.
     * 
     * \param v   Enable, default = true
     */
    void SetForwardSystemMessages(bool v=true) {
        if (v) { ForwardMode |= FwdModeBit_SystemMessages;  } else { ForwardMode &= ~FwdModeBit_SystemMessages; }
      }

    /*********************************************************************//**
     * \brief Enable Only Known Messages for forwarding 
     *
     * Set false as default. With this you can control if unknown messages 
     * will be forwarded to forward stream. If you set this true, all 
     * unknown message will not be forwarded to the stream.
     * 
     * \note This does not effect for own messages. Known messages are 
     * listed on library.  
     * 
     * See \ref tNMEA2000::SetSingleFrameMessages, 
     * \ref tNMEA2000::SetFastPacketMessages, 
     * \ref tNMEA2000::ExtendSingleFrameMessages and 
     * \ref tNMEA2000::ExtendFastPacketMessages.
     * 
     * \param v   Enable, default = false
     */      
    void SetForwardOnlyKnownMessages(bool v=true) {
        if (v) { ForwardMode |= FwdModeBit_OnlyKnownMessages;  } else { ForwardMode &= ~FwdModeBit_OnlyKnownMessages; }
      }

    /*********************************************************************//**
     * \brief Enable Own Messages for forwarding 
     *
     * Set true as default. With this you can control if messages your 
     * device sends to bus will be forwarded to forward stream.
     * 
     * \param v   Enable, default = true
     */
    void SetForwardOwnMessages(bool v=true) {
        if (v) { ForwardMode |= FwdModeBit_OwnMessages;  } else { ForwardMode &= ~FwdModeBit_OwnMessages; }
      }
    
    /*********************************************************************//**
     * \brief Set the Handle Only Known Messages
     *
     * Set false as default. With this you can control if unknown messages 
     * will be handled at all. Known messages are listed on library.
     * 
     * See \ref tNMEA2000::SetSingleFrameMessages, 
     * \ref tNMEA2000::SetFastPacketMessages,
     * \ref tNMEA2000::ExtendSingleFrameMessages and 
     * \ref tNMEA2000::ExtendFastPacketMessages.
     * 
     * \param v   Enable, default = false
     */
    void SetHandleOnlyKnownMessages(bool v=true) {
        if (v) { ForwardMode |= HandleModeBit_OnlyKnownMessages;  } else { ForwardMode &= ~HandleModeBit_OnlyKnownMessages; }
      }

    /*********************************************************************//**
     * \brief Set the Debug Mode of the system
     *
     * If you do not have physical N2k bus connection and you like to test 
     * your board without even CAN controller, you can use this function.  
     * 
     * see \sa Debug mode.
     * 
     * \param _dbMode Debug mode, see \ref tDebugMode
     */
    void SetDebugMode(tDebugMode _dbMode);

    /*********************************************************************//**
     * \brief Checks if the given Address is a broadcast address
     *
     * \param Source  Source address
     * \return true 
     * \return false 
     */
    static bool IsBroadcast(unsigned char Source) { return Source==0xff; }

};

/************************************************************************//**
 * \brief Setting up PGN 59392 Message "ISO Acknowledgement"
 *
 * This message is provided by ISO 11783 for a handshake mechanism 
 * between transmitting and receiving devices. This message 
 * is the possible response to acknowledge the reception of a “normal 
 * broadcast” message or the response to a specific command to indicate 
 * compliance or failure.
 * 
 * \param N2kMsg          Reference to a N2kMsg Object, ready to be send 
 * \param Control         Control Byte
 * \param GroupFunction   Group Function value
 * \param PGN             PGN of requested Information
 */
void SetN2kPGN59392(tN2kMsg &N2kMsg, unsigned char Control, unsigned char GroupFunction, unsigned long PGN);

/************************************************************************//**
 * \brief Setting up Message "ISO Acknowledgement" - PGN 59392
 * 
 * Alias of PGN 59392. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN59392
 */
inline void SetN2kPGNISOAcknowledgement(tN2kMsg &N2kMsg, unsigned char Control, unsigned char GroupFunction, unsigned long PGN) {
  SetN2kPGN59392(N2kMsg,Control,GroupFunction,PGN);
}

/************************************************************************//**
 * \brief Setting up PGN 60928 Message "ISO Address Claim"
 *
 * This network management message is used to claim a network address and 
 * to respond with device information (NAME) requested by the ISO Request 
 * or Complex Request Group Function. This PGN contains several fields 
 * that are Request Parameters that can be used to control the expected 
 * response to requests for this PGN.
 * 
 * 
 * \param N2kMsg            Reference to a N2kMsg Object, ready to be send 
 * \param UniqueNumber      Unique Number (ISO Identity Number)
 * \param ManufacturerCode  Manufacturer Code
 * \param DeviceFunction    Device Function (ISO Function)
 * \param DeviceClass       Device Class
 * \param DeviceInstance    Device Instance
 * \param SystemInstance    System Instance (ISO Device Class Instance)
 * \param IndustryGroup     Industry Group
 */
void SetN2kPGN60928(tN2kMsg &N2kMsg, unsigned long UniqueNumber, int ManufacturerCode,
                   unsigned char DeviceFunction, unsigned char DeviceClass,
                   unsigned char DeviceInstance=0, unsigned char SystemInstance=0, unsigned char IndustryGroup=4
                   );
/************************************************************************//**
 * \brief Setting up PGN 60928 Message "ISO Address Claim"
 *
 * This network management message is used to claim a network address and 
 * to respond with device information (NAME) requested by the ISO Request 
 * or Complex Request Group Function. This PGN contains several fields 
 * that are Request Parameters that can be used to control the expected 
 * response to requests for this PGN.
 * 
 * \param N2kMsg            Reference to a N2kMsg Object, ready to be send 
 * \param Name              Name of the device 
 */
void SetN2kPGN60928(tN2kMsg &N2kMsg, uint64_t Name);

/************************************************************************//**
 * \brief Setting up Message "ISO Address Claim" - PGN 60928
 * 
 * Alias of PGN 60928. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN60928
 */
inline void SetN2kISOAddressClaim(tN2kMsg &N2kMsg, unsigned long UniqueNumber, int ManufacturerCode,
                   unsigned char DeviceFunction, unsigned char DeviceClass,
                   unsigned char DeviceInstance=0, unsigned char SystemInstance=0, unsigned char IndustryGroup=4
                   ) {
  SetN2kPGN60928(N2kMsg, UniqueNumber, ManufacturerCode, DeviceFunction, DeviceClass,
                 DeviceInstance, SystemInstance, IndustryGroup);
}
/************************************************************************//**
 * \brief Setting up Message "ISO Address Claim" - PGN 60928
 * 
 * Alias of PGN 60928. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN60928
 */
inline void SetN2kISOAddressClaim(tN2kMsg &N2kMsg, uint64_t Name) {
  SetN2kPGN60928(N2kMsg, Name);
}

/************************************************************************//**
 * \brief Setting up PGN 126996 Message "Product information"
 *
 * Provides product information onto the network that could be important 
 * for determining quality of data coming from this product.
 * 
 * \param N2kMsg              Reference to a N2kMsg Object, ready to be send 
 * \param N2kVersion          NMEA Network Message Database Version
 * \param ProductCode         NMEA Manufacturer's Product Code
 * \param ModelID             Manufacturer's Model ID
 * \param SwCode              Manufacturer's Software Version Code
 * \param ModelVersion        Manufacturer's Model Version
 * \param ModelSerialCode     Manufacturer's Model Serial Code
 * \param CertificationLevel  NMEA 2000 Certification Level
 * \param LoadEquivalency     Load Equivalency
 */
void SetN2kPGN126996(tN2kMsg &N2kMsg, unsigned int N2kVersion, unsigned int ProductCode,
                     const char *ModelID, const char *SwCode,
                     const char *ModelVersion, const char *ModelSerialCode,
                     unsigned char CertificationLevel=1, unsigned char LoadEquivalency=1);

/************************************************************************//**
 * \brief Setting up Message "Product information" - PGN 126996
 * 
 * Alias of PGN 126996. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN126996
 */
inline void SetN2kProductInformation(tN2kMsg &N2kMsg, unsigned int N2kVersion, unsigned int ProductCode,
                     const char *ModelID, const char *SwCode,
                     const char *ModelVersion, const char *ModelSerialCode,
                     unsigned char CertificationLevel=1, unsigned char LoadEquivalency=1) {
  SetN2kPGN126996(N2kMsg,N2kVersion,ProductCode,
                  ModelID,SwCode,ModelVersion,ModelSerialCode,
                  CertificationLevel,LoadEquivalency);
}

/************************************************************************//**
 * \brief  Parsing the content of message PGN 126996 "Product information"
 *
 * Provides product information onto the network that could be important 
 * for determining quality of data coming from this product.
 * 
 * \param N2kMsg              Reference to a N2kMsg Object, ready to be send 
 * \param N2kVersion          NMEA Network Message Database Version
 * \param ProductCode         NMEA Manufacturer's Product Code
 * \param ModelIDSize         Size of ModelID
 * \param ModelID             Manufacturer's Model ID
 * \param SwCodeSize          Size of Software Version Code
 * \param SwCode              Manufacturer's Software Version Code
 * \param ModelVersionSize    Size of Model Version
 * \param ModelVersion        Manufacturer's Model Version
 * \param ModelSerialCodeSize Size of Model Serial Code
 * \param ModelSerialCode     Manufacturer's Model Serial Code
 * \param CertificationLevel  NMEA 2000 Certification Level
 * \param LoadEquivalency     Load Equivalency
 * 
 * \return true     Parsing of PGN Message successful
 * \return false    Parsing of PGN Message aborted 
 */
bool ParseN2kPGN126996(const tN2kMsg& N2kMsg, unsigned short &N2kVersion, unsigned short &ProductCode,
                     int ModelIDSize, char *ModelID, int SwCodeSize, char *SwCode,
                     int ModelVersionSize, char * ModelVersion, int ModelSerialCodeSize, char *ModelSerialCode,
                     unsigned char &CertificationLevel, unsigned char &LoadEquivalency);

/************************************************************************//**
 * \brief Setting up PGN 126998 Message "Configuration information"
 *
 * Free-form alphanumeric fields describing the installation (e.g., 
 * starboard engine room location) of the device and installation 
 * notes (e.g., calibration data).
 * 
 * \param N2kMsg              Reference to a N2kMsg Object, ready to be send 
 * \param ManufacturerInformation   Manufacturer Information
 * \param InstallationDescription1  Installation Description, Field 1
 * \param InstallationDescription2  Installation Description, Field 2
 * \param UsePgm              Use program memory, default = false
 */
void SetN2kPGN126998(tN2kMsg &N2kMsg,
                     const char *ManufacturerInformation,
                     const char *InstallationDescription1=0,
                     const char *InstallationDescription2=0,
                     bool UsePgm=false);

/************************************************************************//**
 * \brief Setting up Message "Configuration information" - PGN 126998
 * 
 * Alias of PGN 126998. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN126998
 */
inline void SetN2kConfigurationInformation(tN2kMsg &N2kMsg,
                     const char *ManufacturerInformation,
                     const char *InstallationDescription1=0,
                     const char *InstallationDescription2=0,
                     bool UsePgm=false) {
  SetN2kPGN126998(N2kMsg,
                  ManufacturerInformation,
                  InstallationDescription1,
                  InstallationDescription2,
                  UsePgm);
}

/************************************************************************//**
 * \brief Parsing the content of message PGN 126998 
 *    "Configuration information"
 *
 * Free-form alphanumeric fields describing the installation (e.g., 
 * starboard engine room location) of the device and installation 
 * notes (e.g., calibration data).
 *
 * \param N2kMsg            Reference to a N2kMsg Object
 * \param ManufacturerInformationSize Size off Manufacturer Information
 * \param ManufacturerInformation     Manufacturer Information
 * \param InstallationDescription1Size Size off Installation Description
 * \param InstallationDescription1    Installation Description, Field 1
 * \param InstallationDescription2Size Size off Installation Description
 * \param InstallationDescription2    Installation Description, Field 2
 * 
 * \return true     Parsing of PGN Message successful
 * \return false    Parsing of PGN Message aborted 
 * 
 */
bool ParseN2kPGN126998(const tN2kMsg& N2kMsg,
                       size_t &ManufacturerInformationSize, char *ManufacturerInformation,
                       size_t &InstallationDescription1Size, char *InstallationDescription1,
                       size_t &InstallationDescription2Size, char *InstallationDescription2);

/************************************************************************//**
 * \brief Setting up PGN 59904 Message "ISO request"
 *
 * As defined by ISO, this message has a data length of 3 bytes with no 
 * padding added to complete the single frame. The appropriate response 
 * to this message is based on the PGN being requested, and whether the 
 * receiver supports the requested PGN.
 * 
 * \param N2kMsg            Reference to a N2kMsg Object, ready to be send 
 * \param Destination       Address of the destination
 * \param RequestedPGN      PGN being requested
 */
void SetN2kPGN59904(tN2kMsg &N2kMsg, uint8_t Destination, unsigned long RequestedPGN);

/************************************************************************//**
 * \brief Setting up Message "ISO request" - PGN 59904
 * 
 * Alias of PGN 59904. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN59904
 */
inline void SetN2kPGNISORequest(tN2kMsg &N2kMsg, uint8_t Destination, unsigned long RequestedPGN) {
  SetN2kPGN59904(N2kMsg,Destination,RequestedPGN);
}

/************************************************************************//**
 * \brief Parsing the content of message PGN 59904 "ISO request"
 *
 * As defined by ISO, this message has a data length of 3 bytes with no 
 * padding added to complete the single frame. The appropriate response 
 * to this message is based on the PGN being requested, and whether the 
 * receiver supports the requested PGN.
 * 
 * \param N2kMsg        Reference to a N2kMsg Object 
 * \param RequestedPGN  PGN being requested
 * 
 * \return true     Parsing of PGN Message successful
 * \return false    Parsing of PGN Message aborted 
 */
bool ParseN2kPGN59904(const tN2kMsg &N2kMsg, unsigned long &RequestedPGN);

/************************************************************************//**
 * \brief Parsing the content of a "ISO request" 
 *        message - PGN 59904
 * 
 * Alias of PGN 59904. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref ParseN2kPGN59904 
 */
inline bool ParseN2kPGNISORequest(const tN2kMsg &N2kMsg, unsigned long &RequestedPGN) {
  return ParseN2kPGN59904(N2kMsg, RequestedPGN);
}

/************************************************************************//**
 * \enum    tN2kPGNList
 * \brief   Enumeration of types for PGN lists according to PGN 126464
 *
 */
enum tN2kPGNList {N2kpgnl_transmit=0, N2kpgnl_receive=1 };

/************************************************************************//**
 * \brief Setting up PGN 126464 Message "PGN List - Transmit PGNs group 
 *        function"
 * 
 * The PGN List group function type is defined by the first field. The 
 * message will be either a Transmit PGNs or a Receive PGNs group function 
 * that identifies the PGNs transmitted from or received by a node.
 * 
 * \note List of PGNs must be null terminated and defined as PROGMEM e.g. 
 *        const unsigned long TransmitMessages[] PROGMEM={130310L,0};
 * 
 * \param N2kMsg          Reference to a N2kMsg Object, ready to be send 
 * \param Destination     Address of the destination
 * \param tr              Transmit or Receive, see \ref tN2kPGNList
 * \param PGNs            List of PGNs
 */
void SetN2kPGN126464(tN2kMsg &N2kMsg, uint8_t Destination, tN2kPGNList tr, const unsigned long *PGNs);

/************************************************************************//**
 * \brief Setting up Message "PGN List - Transmit PGNs group 
 *        function" - PGN 126464
 * 
 * Alias of PGN 126464. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN126464
 */
inline void SetN2kPGNTransmitList(tN2kMsg &N2kMsg, uint8_t Destination, const unsigned long *PGNs) {
  SetN2kPGN126464(N2kMsg,Destination,N2kpgnl_transmit,PGNs);
}

/************************************************************************//**
 * \brief Setting up PGN 126993 Message "Heartbeat"
 * 
 * This PGN shall be transmitted by all NMEA devices. Reception of this 
 * PGN confirms that a device is still present on the network. Reception 
 * of this PGN may also be used to maintain an address to NAME association 
 * table within the receiving device.
 * 
 * \param N2kMsg            Reference to a N2kMsg Object, ready to be send
 * \param timeInterval_ms   time interval in msec (0.01 - 655.32s )
 * \param sequenceCounter   Sequence counter
 */
void SetN2kPGN126993(tN2kMsg &N2kMsg, uint32_t timeInterval_ms, uint8_t sequenceCounter);

/************************************************************************//**
 * \brief Setting up Message "Heartbeat" - PGN 126993
 * 
 * Alias of PGN 126993. This alias was introduced to improve the readability
 * of the source code. See parameter details on \ref SetN2kPGN126993
 */
inline void SetHeartbeat(tN2kMsg &N2kMsg, uint32_t timeInterval_ms, uint8_t sequenceCounter) {
	SetN2kPGN126993(N2kMsg, timeInterval_ms, sequenceCounter);
}

#endif
