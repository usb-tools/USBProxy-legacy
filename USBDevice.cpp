#include <linux/types.h>
#include "USBDevice.h"
#include "Packets.h"
#include <memory.h>

USBDevice::USBDevice(USBDeviceProxy* proxy) {
	__u8 buf[18];
	SETUP_PACKET setup_packet;
	setup_packet.bmRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_DEVICE;
	setup_packet.wIndex=0;
	setup_packet.wLength=18;
	__u16 len=0;
	proxy->control_request(&setup_packet,&len,buf);
	memcpy(&descriptor,buf,len);
}

USBDevice::USBDevice(usb_device_descriptor _descriptor) {
	descriptor=_descriptor;
}

USBDevice::USBDevice(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations) {
	descriptor.bcdUSB=bcdUSB;
	descriptor.bDeviceClass=bDeviceClass;
	descriptor.bDeviceSubClass=bDeviceSubClass;
	descriptor.bDeviceProtocol=bDeviceProtocol;
	descriptor.bMaxPacketSize0=bMaxPacketSize0;
	descriptor.idVendor=idVendor;
	descriptor.idProduct=idProduct;
	descriptor.bcdDevice=bcdDevice;
	descriptor.iManufacturer=iManufacturer;
	descriptor.iProduct=iProduct;
	descriptor.iSerialNumber=iSerialNumber;
	descriptor.bNumConfigurations=bNumConfigurations;
}

usb_device_descriptor USBDevice::getDescriptor() {
	return descriptor;
};

/*

    Public ReadOnly Property get_request_handlers As Dictionary(Of Byte, Action(Of USBDeviceRequest)) Implements IRequest_Handler.get_request_handlers
        Get
            Return request_handlers
        End Get
    End Property

    Public name As String = "generic_device"
    Public maxusb_app As MAXUSBApp
    Public verbose As Byte
    Public strings As New List(Of String)

    Public usb_spec_version As UInt16
    Public device_class As Byte
    Public device_subclass As Byte
    Public protocol_rel_num As Byte
    Public max_packet_size_ep0 As Byte
    Public vendor_id As UInt16
    Public product_id As UInt16
    Public device_rev As UInt16
    Public manufacturer_string_id As UInt16
    Public product_string_id As UInt16
    Public serial_number_string_id As UInt16

    Public config_num As Short
    Public configuration As USBConfiguration
    Public state As USB.state
    Public ready As Boolean
    Public address As Byte
    Public device_vendor As USBVendor

    Public configurations As New Dictionary(Of Byte, USBConfiguration)
    Public endpoints As New Dictionary(Of Byte, USBEndpoint)

    Public request_handlers As New Dictionary(Of Byte, Action(Of USBDeviceRequest))
    Public descriptors As New Dictionary(Of USB.desc_type, Func(Of USBDeviceRequest, Byte()))

    Public Sub New()
    End Sub


    Public Overridable ReadOnly Property get_device_class As USBClass Implements IRequest_Recipient.get_device_class
        Get
            Return Nothing
        End Get
    End Property

    Public Overridable ReadOnly Property get_device_vendor As USBVendor Implements IRequest_Recipient.get_device_vendor
        Get
            Return device_vendor
        End Get
    End Property

    Public Sub New(maxusb_app As MAXUSBApp, device_class As Byte, device_subclass As Byte, protocol_rel_num As Byte, max_packet_size_ep0 As Byte, _
                   vendor_id As UInt16, product_id As UInt16, device_rev As UInt16, manufacturer_string As String, product_string As String, _
                   serial_number_string As String, Optional configurations() As USBConfiguration = Nothing, Optional descriptors As Dictionary(Of USB.desc_type, Func(Of USBDeviceRequest, Byte())) = Nothing, _
                   Optional verbose As Byte = 0)
        Me.maxusb_app = maxusb_app
        Me.verbose = verbose
        Me.strings = New List(Of String)
        Me.usb_spec_version = &H1
        Me.device_class = device_class
        Me.device_subclass = device_subclass
        Me.protocol_rel_num = protocol_rel_num
        Me.max_packet_size_ep0 = max_packet_size_ep0
        Me.vendor_id = vendor_id
        Me.product_id = product_id
        Me.device_rev = device_rev
        Me.device_vendor = Nothing

        Me.manufacturer_string_id = Me.get_string_id(manufacturer_string)
        Me.product_string_id = Me.get_string_id(product_string)
        Me.serial_number_string_id = Me.get_string_id(serial_number_string)
        If Not descriptors Is Nothing Then Me.descriptors = descriptors
        If Me.descriptors.ContainsKey(USB.desc_type._device) Then Me.descriptors(USB.desc_type._device) = AddressOf Me.get_descriptor Else Me.descriptors.Add(USB.desc_type._device, AddressOf Me.get_descriptor)
        If Me.descriptors.ContainsKey(USB.desc_type._configuration) Then Me.descriptors(USB.desc_type._configuration) = AddressOf Me.handle_get_configuration_descriptor_request Else Me.descriptors.Add(USB.desc_type._configuration, AddressOf Me.handle_get_configuration_descriptor_request)
        If Me.descriptors.ContainsKey(USB.desc_type._string) Then Me.descriptors(USB.desc_type._string) = AddressOf Me.handle_get_string_descriptor_request Else Me.descriptors.Add(USB.desc_type._string, AddressOf Me.handle_get_string_descriptor_request)

        Me.config_num = -1
        Me.configuration = Nothing
        If Not configurations Is Nothing Then
            For Each c As USBConfiguration In configurations
                Me.configurations.Add(c.configuration_index, c)
            Next
        End If

        For Each c As USBConfiguration In configurations
            Dim csi As UInt16 = Me.get_string_id(c.configuration_string)
            c.set_configuration_string_index(csi)
            c.set_device(Me)
        Next c

        Me.state = USB.state._detached
        Me.ready = False
        Me.address = 0
        Me.setup_request_handlers()
    End Sub

    Public Function get_string_id(s As String)
        If strings.Contains(s) Then
            Return strings.IndexOf(s) + 1
        Else
            strings.Add(s)
            Return strings.Count
        End If
    End Function

    Public Sub setup_request_handlers()
        request_handlers.Add(0, AddressOf handle_get_status_request)
        request_handlers.Add(1, AddressOf handle_clear_feature_request)
        request_handlers.Add(3, AddressOf handle_set_feature_request)
        request_handlers.Add(5, AddressOf handle_set_address_request)
        request_handlers.Add(6, AddressOf handle_get_descriptor_request)
        request_handlers.Add(7, AddressOf handle_set_descriptor_request)
        request_handlers.Add(8, AddressOf handle_get_configuration_request)
        request_handlers.Add(9, AddressOf handle_set_configuration_request)
        request_handlers.Add(10, AddressOf handle_get_interface_request)
        request_handlers.Add(11, AddressOf handle_set_interface_request)
        request_handlers.Add(12, AddressOf handle_synch_frame_request)
    End Sub

    Public Sub connect()
        Me.maxusb_app.connect(Me)
        Me.state = USB.state._powered
    End Sub

    Public Overridable Sub disconnect()
        Me.maxusb_app.disconnect()
        Me.state = USB.state._detached
    End Sub

    Public Sub run()
        Me.maxusb_app.service_irqs()
    End Sub

    Public Sub ack_status_stage()
        Me.maxusb_app.ack_status_stage()
    End Sub

    Public Function get_descriptor(req As USBDeviceRequest) As Byte()
        Dim d() As Byte = New Byte() { _
            18, _
            1, _
            Me.usb_spec_version \ 256 And 255, _
            Me.usb_spec_version And 255, _
            Me.device_class, _
            Me.device_subclass, _
            Me.protocol_rel_num, _
            Me.max_packet_size_ep0, _
            Me.vendor_id And 255, _
            Me.vendor_id \ 256 And 255, _
            Me.product_id And 255, _
            Me.product_id \ 256 And 255, _
            Me.device_rev And 255, _
            Me.device_rev \ 256 And 255, _
            Me.manufacturer_string_id, _
            Me.product_string_id, _
            Me.serial_number_string_id, _
            Me.configurations.Count}
        Return d
    End Function

    Public Sub handle_request(req As USBDeviceRequest)
        Dim recipient As IRequest_Recipient = Nothing
        Dim handler_entity As IRequest_Handler = Nothing
        Dim handler As Action(Of USBDeviceRequest) = Nothing
        Dim req_type As USB.request_type = req.get_type()

        If Me.verbose > 3 Then Debug.Print(Me.name & " received request " & req.ToString())
        Dim recipient_type As USB.request_recipient = req.get_recipient()
        Dim index As UInt16 = req.get_index() And &HFF
        Select Case recipient_type
            Case USB.request_recipient._device
                recipient = Me
            Case USB.request_recipient._interface
                If Me.configuration.interfaces.ContainsKey(index) Then recipient = Me.configuration.interfaces(index)
            Case USB.request_recipient._endpoint
                recipient = endpoints(index)
        End Select

        Select Case req_type
            Case USB.request_type._standard
                handler_entity = recipient
            Case USB.request_type._class
                handler_entity = recipient.get_device_class
            Case USB.request_type._vendor
                handler_entity = recipient.get_device_vendor
        End Select

        If recipient Is Nothing Then
            Debug.Print(Me.name & " invalid recipient, stalling")
            Me.maxusb_app.stall_ep0()
            Exit Sub
        End If

        If handler_entity Is Nothing Then
            Debug.Print(Me.name & " invalid handler entity, stalling")
            Me.maxusb_app.stall_ep0()
            Exit Sub
        End If
        handler_entity.get_request_handlers.TryGetValue(req.request, handler)


        If handler Is Nothing Then
            Debug.Print(Me.name & " invalid handler, stalling")
            Me.maxusb_app.stall_ep0()
            Exit Sub
        End If

        handler(req)
    End Sub

    Sub handle_data_available(ep_num As Byte, data() As Byte)
        If Me.state = USB.state._configured AndAlso Me.endpoints.ContainsKey(ep_num) Then
            Dim endpoint As USBEndpoint = Me.endpoints(ep_num)
            If Not endpoint.handler Is Nothing Then endpoint.handler(data)
        End If
    End Sub

    Sub handle_buffer_available(ep_num As Byte)
        If Me.state = USB.state._configured AndAlso Me.endpoints.ContainsKey(ep_num) Then
            Dim endpoint As USBEndpoint = Me.endpoints(ep_num)
            If Not endpoint.handler Is Nothing Then endpoint.handler(Nothing)
        End If
    End Sub

    Sub handle_get_status_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received GET_STATUS request")
        Dim response() As Byte = New Byte() {&H3, &H0}
        Me.maxusb_app.send_on_endpoint(0, response, Me.max_packet_size_ep0)
    End Sub

    Sub handle_clear_feature_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received CLEAR_FEATURE request with type 0x" & req.request_type.ToString("X2") & " and value 0x" & req.value.ToString("X2"))
        Me.ack_status_stage()
    End Sub

    Sub handle_set_feature_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received SET_FEATURE request")
    End Sub

    Sub handle_set_address_request(req As USBDeviceRequest)
        Me.address = req.value
        Me.state = USB.state._address
        Me.ack_status_stage()
        If Me.verbose > 2 Then Debug.Print(Me.name & " received SET_ADDRESS request for address " & Me.address)
    End Sub

    Sub handle_get_descriptor_request(req As USBDeviceRequest)
        Dim dtype As USB.desc_type = req.value \ 256 And 255
        Dim dindex As Byte = req.value And 255
        Dim lang As UInt16 = req.index
        Dim n As UInt16 = req.length
        Dim response() As Byte = Nothing
        If Me.vendor_id > 2 Then Debug.Print(Me.name & " received GET_DESCRIPTOR req " & dtype & ", index " & dindex & ", language 0x" & lang.ToString("X4") & ", length " & n)

        Dim responseFunc As Func(Of USBDeviceRequest, Byte()) = Nothing
        Me.descriptors.TryGetValue(dtype, responseFunc)
        If Not responseFunc Is Nothing Then response = responseFunc(req)

        If response Is Nothing Then
            Me.maxusb_app.stall_ep0()
        Else
            n = Math.Min(n, response.Length)
            Dim tmp(n - 1) As Byte
            Array.Copy(response, 0, tmp, 0, n)
            Me.maxusb_app.verbose = Me.maxusb_app.verbose + 1
            Me.maxusb_app.send_on_endpoint(0, tmp, Me.max_packet_size_ep0)
            Me.maxusb_app.verbose = Me.maxusb_app.verbose - 1
            If Me.verbose > 5 Then Debug.Print(Me.name & " sent " & n & " bytes in response")
        End If
    End Sub

    Function handle_get_configuration_descriptor_request(req As USBDeviceRequest) As Byte()
        Return Me.configurations((req.value And 255) + 1).get_descriptor(0)
    End Function

    Function handle_get_string_descriptor_request(req As USBDeviceRequest) As Byte()
        Dim num As Byte = req.value And 255
        If num = 0 Then
            '# HACK: hard-coding baaaaad
            Return New Byte() {4, 3, 9, 4}
        Else
            Dim s() As Byte = Unicode.GetBytes(Me.strings(num - 1))
            Dim d(s.Length + 1) As Byte
            d(0) = d.Length
            d(1) = 3
            Array.Copy(s, 0, d, 2, s.Length)
            Return d
        End If
    End Function

    Sub handle_set_descriptor_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received SET_DESCRIPTOR request")
    End Sub

    Sub handle_get_configuration_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received GET_CONFIGURATION request with data 0x" & req.value.ToString("X4"))
    End Sub

    Overridable Sub handle_set_configuration_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received SET_CONFIGURATION request")
        Me.config_num = req.value
        Me.configuration = Me.configurations(Me.config_num)
        Me.state = USB.state._configured
        Me.endpoints = New Dictionary(Of Byte, USBEndpoint)
        For Each i As USBInterface In Me.configuration.interfaces.Values
            For Each e As USBEndpoint In i.endpoints.Values
                Dim epAddr As Byte = e.number + +IIf(e.direction, 128, 0)
                If Not endpoints.ContainsKey(epAddr) Then endpoints.Add(epAddr, e)
            Next
        Next
        Me.ack_status_stage()
    End Sub

    Sub handle_get_interface_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received GET_INTERFACE request")
        '            # HACK: currently only support one interface
        If req.index = 0 Then
            Me.maxusb_app.send_on_endpoint(0, New Byte() {&H0}, Me.max_packet_size_ep0)
        Else
            Me.maxusb_app.stall_ep0()
        End If
    End Sub

    Sub handle_set_interface_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received SET_INTERFACE request")
    End Sub

    Sub handle_synch_frame_request(req As USBDeviceRequest)
        Debug.Print(Me.name & " received SYNCH_FRAME request")
    End Sub

End Class

Public Class USBDeviceRequest
    Public request_type As Byte
    Public request As Byte
    Public value As UInt16
    Public index As UInt16
    Public length As UInt16

    Public Sub New(buf() As Byte)
        Me.request_type = buf(0)
        Me.request = buf(1)
        Me.value = buf(2) + buf(3) * 256
        Me.index = buf(4) + buf(5) * 256
        Me.length = buf(6) + buf(7) * 256
        'If buf(0) <> 128 And buf(0) <> 0 Then Stop

    End Sub

    Public Overrides Function ToString() As String
        Return "dir=" & Me.get_direction & ", type=" & Me.get_type & ", rec=" & Me.get_recipient & ", r=" & Me.request & ", v=" & Me.value & ", i=" & Me.index & ", l=" & Me.length
    End Function

    Public Function raw() As Byte()
        Return New Byte() {Me.request_type, Me.request, Me.value And 255, (Me.value \ 256) And 255, Me.index And 255, (Me.index \ 256) And 255, Me.length And 255, (Me.length \ 256) And 255}
    End Function

    Public Function get_direction() As Byte
        Return (Me.request_type \ 128) And &H1
    End Function

    Public Function get_type() As Byte
        Return (Me.request_type \ 32) And &H3
    End Function

    Public Function get_recipient() As Byte
        Return Me.request_type And &H1F
    End Function

    Public Function get_index() As UInt16
        Dim rec As Byte = Me.get_recipient
        Select Case rec
            Case 1
                Return Me.index
            Case 2
                Return Me.index And &HF
        End Select
        Return 0
    End Function
End Class
*/