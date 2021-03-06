/*
 * P2P - IE builder
 * Copyright (c) 2009-2010, Atheros Communications
 * Copyright(c) 2013 - 2014 Intel Mobile Communications GmbH.
 * Copyright(c) 2011 - 2014 Intel Corporation. All rights reserved.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "common/ieee802_11_defs.h"
#include "wps/wps_i.h"
#include "p2p_i.h"


void p2p_buf_add_action_hdr(struct wpabuf *buf, u8 subtype, u8 dialog_token)
{
	wpabuf_put_u8(buf, WLAN_ACTION_VENDOR_SPECIFIC);
	wpabuf_put_be32(buf, P2P_IE_VENDOR_TYPE);

	wpabuf_put_u8(buf, subtype); /* OUI Subtype */
	wpabuf_put_u8(buf, dialog_token);
	wpa_printf(MSG_DEBUG, "P2P: * Dialog Token: %d", dialog_token);
}


void p2p_buf_add_public_action_hdr(struct wpabuf *buf, u8 subtype,
				   u8 dialog_token)
{
	wpabuf_put_u8(buf, WLAN_ACTION_PUBLIC);
	wpabuf_put_u8(buf, WLAN_PA_VENDOR_SPECIFIC);
	wpabuf_put_be32(buf, P2P_IE_VENDOR_TYPE);

	wpabuf_put_u8(buf, subtype); /* OUI Subtype */
	wpabuf_put_u8(buf, dialog_token);
	wpa_printf(MSG_DEBUG, "P2P: * Dialog Token: %d", dialog_token);
}


u8 * p2p_buf_add_ie_hdr(struct wpabuf *buf)
{
	u8 *len;

	/* P2P IE header */
	wpabuf_put_u8(buf, WLAN_EID_VENDOR_SPECIFIC);
	len = wpabuf_put(buf, 1); /* IE length to be filled */
	wpabuf_put_be32(buf, P2P_IE_VENDOR_TYPE);
	wpa_printf(MSG_DEBUG, "P2P: * P2P IE header");
	return len;
}


void p2p_buf_update_ie_hdr(struct wpabuf *buf, u8 *len)
{
	/* Update P2P IE Length */
	*len = (u8 *) wpabuf_put(buf, 0) - len - 1;
}


void p2p_buf_add_capability(struct wpabuf *buf, u8 dev_capab, u8 group_capab)
{
	/* P2P Capability */
	wpabuf_put_u8(buf, P2P_ATTR_CAPABILITY);
	wpabuf_put_le16(buf, 2);
	wpabuf_put_u8(buf, dev_capab); /* Device Capabilities */
	wpabuf_put_u8(buf, group_capab); /* Group Capabilities */
	wpa_printf(MSG_DEBUG, "P2P: * Capability dev=%02x group=%02x",
		   dev_capab, group_capab);
}


void p2p_buf_add_go_intent(struct wpabuf *buf, u8 go_intent)
{
	/* Group Owner Intent */
	wpabuf_put_u8(buf, P2P_ATTR_GROUP_OWNER_INTENT);
	wpabuf_put_le16(buf, 1);
	wpabuf_put_u8(buf, go_intent);
	wpa_printf(MSG_DEBUG, "P2P: * GO Intent: Intent %u Tie breaker %u",
		   go_intent >> 1, go_intent & 0x01);
}


void p2p_buf_add_listen_channel(struct wpabuf *buf, const char *country,
				u8 reg_class, u8 channel)
{
	/* Listen Channel */
	wpabuf_put_u8(buf, P2P_ATTR_LISTEN_CHANNEL);
	wpabuf_put_le16(buf, 5);
	wpabuf_put_data(buf, country, 3);
	wpabuf_put_u8(buf, reg_class); /* Regulatory Class */
	wpabuf_put_u8(buf, channel); /* Channel Number */
	wpa_printf(MSG_DEBUG, "P2P: * Listen Channel: Regulatory Class %u "
		   "Channel %u", reg_class, channel);
}


void p2p_buf_add_operating_channel(struct wpabuf *buf, const char *country,
				   u8 reg_class, u8 channel)
{
	/* Operating Channel */
	wpabuf_put_u8(buf, P2P_ATTR_OPERATING_CHANNEL);
	wpabuf_put_le16(buf, 5);
	wpabuf_put_data(buf, country, 3);
	wpabuf_put_u8(buf, reg_class); /* Regulatory Class */
	wpabuf_put_u8(buf, channel); /* Channel Number */
	wpa_printf(MSG_DEBUG, "P2P: * Operating Channel: Regulatory Class %u "
		   "Channel %u", reg_class, channel);
}


void p2p_buf_add_channel_list(struct wpabuf *buf, const char *country,
			      struct p2p_channels *chan)
{
	u8 *len;
	size_t i;

	/* Channel List */
	wpabuf_put_u8(buf, P2P_ATTR_CHANNEL_LIST);
	len = wpabuf_put(buf, 2); /* IE length to be filled */
	wpabuf_put_data(buf, country, 3); /* Country String */

	for (i = 0; i < chan->reg_classes; i++) {
		struct p2p_reg_class *c = &chan->reg_class[i];
		wpabuf_put_u8(buf, c->reg_class);
		wpabuf_put_u8(buf, c->channels);
		wpabuf_put_data(buf, c->channel, c->channels);
	}

	/* Update attribute length */
	WPA_PUT_LE16(len, (u8 *) wpabuf_put(buf, 0) - len - 2);
	wpa_hexdump(MSG_DEBUG, "P2P: * Channel List",
		    len + 2, (u8 *) wpabuf_put(buf, 0) - len - 2);
}


void p2p_buf_add_status(struct wpabuf *buf, u8 status)
{
	/* Status */
	wpabuf_put_u8(buf, P2P_ATTR_STATUS);
	wpabuf_put_le16(buf, 1);
	wpabuf_put_u8(buf, status);
	wpa_printf(MSG_DEBUG, "P2P: * Status: %d", status);
}


void p2p_buf_add_device_info(struct wpabuf *buf, struct p2p_data *p2p,
			     struct p2p_device *peer)
{
	u8 *len;
	u16 methods;
	size_t nlen, i;

	/* P2P Device Info */
	wpabuf_put_u8(buf, P2P_ATTR_DEVICE_INFO);
	len = wpabuf_put(buf, 2); /* IE length to be filled */

	/* P2P Device address */
	wpabuf_put_data(buf, p2p->cfg->dev_addr, ETH_ALEN);

	/* Config Methods */
	methods = 0;
	if (peer && peer->wps_method != WPS_NOT_READY) {
		if (peer->wps_method == WPS_PBC)
			methods |= WPS_CONFIG_PUSHBUTTON;
		else if (peer->wps_method == WPS_PIN_DISPLAY ||
			 peer->wps_method == WPS_PIN_KEYPAD) {
			methods |= WPS_CONFIG_DISPLAY | WPS_CONFIG_KEYPAD;
			methods |= WPS_CONFIG_P2PS;
		}
	} else if (p2p->cfg->config_methods) {
		methods |= p2p->cfg->config_methods &
			(WPS_CONFIG_PUSHBUTTON | WPS_CONFIG_DISPLAY |
			 WPS_CONFIG_KEYPAD | WPS_CONFIG_P2PS);
	} else {
		methods |= WPS_CONFIG_PUSHBUTTON;
		methods |= WPS_CONFIG_DISPLAY | WPS_CONFIG_KEYPAD;
		methods |= WPS_CONFIG_P2PS;
	}
	wpabuf_put_be16(buf, methods);

	/* Primary Device Type */
	wpabuf_put_data(buf, p2p->cfg->pri_dev_type,
			sizeof(p2p->cfg->pri_dev_type));

	/* Number of Secondary Device Types */
	wpabuf_put_u8(buf, p2p->cfg->num_sec_dev_types);

	/* Secondary Device Type List */
	for (i = 0; i < p2p->cfg->num_sec_dev_types; i++)
		wpabuf_put_data(buf, p2p->cfg->sec_dev_type[i],
				WPS_DEV_TYPE_LEN);

	/* Device Name */
	nlen = p2p->cfg->dev_name ? os_strlen(p2p->cfg->dev_name) : 0;
	wpabuf_put_be16(buf, ATTR_DEV_NAME);
	wpabuf_put_be16(buf, nlen);
	wpabuf_put_data(buf, p2p->cfg->dev_name, nlen);

	/* Update attribute length */
	WPA_PUT_LE16(len, (u8 *) wpabuf_put(buf, 0) - len - 2);
	wpa_printf(MSG_DEBUG, "P2P: * Device Info");
}


void p2p_buf_add_device_id(struct wpabuf *buf, const u8 *dev_addr)
{
	/* P2P Device ID */
	wpabuf_put_u8(buf, P2P_ATTR_DEVICE_ID);
	wpabuf_put_le16(buf, ETH_ALEN);
	wpabuf_put_data(buf, dev_addr, ETH_ALEN);
	wpa_printf(MSG_DEBUG, "P2P: * Device ID: " MACSTR, MAC2STR(dev_addr));
}


void p2p_buf_add_config_timeout(struct wpabuf *buf, u8 go_timeout,
				u8 client_timeout)
{
	/* Configuration Timeout */
	wpabuf_put_u8(buf, P2P_ATTR_CONFIGURATION_TIMEOUT);
	wpabuf_put_le16(buf, 2);
	wpabuf_put_u8(buf, go_timeout);
	wpabuf_put_u8(buf, client_timeout);
	wpa_printf(MSG_DEBUG, "P2P: * Configuration Timeout: GO %d (*10ms)  "
		   "client %d (*10ms)", go_timeout, client_timeout);
}


void p2p_buf_add_intended_addr(struct wpabuf *buf, const u8 *interface_addr)
{
	/* Intended P2P Interface Address */
	wpabuf_put_u8(buf, P2P_ATTR_INTENDED_INTERFACE_ADDR);
	wpabuf_put_le16(buf, ETH_ALEN);
	wpabuf_put_data(buf, interface_addr, ETH_ALEN);
	wpa_printf(MSG_DEBUG, "P2P: * Intended P2P Interface Address " MACSTR,
		   MAC2STR(interface_addr));
}


void p2p_buf_add_group_bssid(struct wpabuf *buf, const u8 *bssid)
{
	/* P2P Group BSSID */
	wpabuf_put_u8(buf, P2P_ATTR_GROUP_BSSID);
	wpabuf_put_le16(buf, ETH_ALEN);
	wpabuf_put_data(buf, bssid, ETH_ALEN);
	wpa_printf(MSG_DEBUG, "P2P: * P2P Group BSSID " MACSTR,
		   MAC2STR(bssid));
}


void p2p_buf_add_group_id(struct wpabuf *buf, const u8 *dev_addr,
			  const u8 *ssid, size_t ssid_len)
{
	/* P2P Group ID */
	wpabuf_put_u8(buf, P2P_ATTR_GROUP_ID);
	wpabuf_put_le16(buf, ETH_ALEN + ssid_len);
	wpabuf_put_data(buf, dev_addr, ETH_ALEN);
	wpabuf_put_data(buf, ssid, ssid_len);
	wpa_printf(MSG_DEBUG, "P2P: * P2P Group ID " MACSTR,
		   MAC2STR(dev_addr));
	wpa_hexdump_ascii(MSG_DEBUG, "P2P: P2P Group ID SSID", ssid, ssid_len);
}


void p2p_buf_add_invitation_flags(struct wpabuf *buf, u8 flags)
{
	/* Invitation Flags */
	wpabuf_put_u8(buf, P2P_ATTR_INVITATION_FLAGS);
	wpabuf_put_le16(buf, 1);
	wpabuf_put_u8(buf, flags);
	wpa_printf(MSG_DEBUG, "P2P: * Invitation Flags: bitmap 0x%x", flags);
}


static void p2p_buf_add_noa_desc(struct wpabuf *buf, struct p2p_noa_desc *desc)
{
	if (desc == NULL)
		return;

	wpabuf_put_u8(buf, desc->count_type);
	wpabuf_put_le32(buf, desc->duration);
	wpabuf_put_le32(buf, desc->interval);
	wpabuf_put_le32(buf, desc->start_time);
}


void p2p_buf_add_noa(struct wpabuf *buf, u8 noa_index, u8 opp_ps, u8 ctwindow,
		     struct p2p_noa_desc *desc1, struct p2p_noa_desc *desc2)
{
	/* Notice of Absence */
	wpabuf_put_u8(buf, P2P_ATTR_NOTICE_OF_ABSENCE);
	wpabuf_put_le16(buf, 2 + (desc1 ? 13 : 0) + (desc2 ? 13 : 0));
	wpabuf_put_u8(buf, noa_index);
	wpabuf_put_u8(buf, (opp_ps ? 0x80 : 0) | (ctwindow & 0x7f));
	p2p_buf_add_noa_desc(buf, desc1);
	p2p_buf_add_noa_desc(buf, desc2);
	wpa_printf(MSG_DEBUG, "P2P: * Notice of Absence");
}


void p2p_buf_add_ext_listen_timing(struct wpabuf *buf, u16 period,
				   u16 interval)
{
	/* Extended Listen Timing */
	wpabuf_put_u8(buf, P2P_ATTR_EXT_LISTEN_TIMING);
	wpabuf_put_le16(buf, 4);
	wpabuf_put_le16(buf, period);
	wpabuf_put_le16(buf, interval);
	wpa_printf(MSG_DEBUG, "P2P: * Extended Listen Timing (period %u msec  "
		   "interval %u msec)", period, interval);
}


void p2p_buf_add_p2p_interface(struct wpabuf *buf, struct p2p_data *p2p)
{
	/* P2P Interface */
	wpabuf_put_u8(buf, P2P_ATTR_INTERFACE);
	wpabuf_put_le16(buf, ETH_ALEN + 1 + ETH_ALEN);
	/* P2P Device address */
	wpabuf_put_data(buf, p2p->cfg->dev_addr, ETH_ALEN);
	/*
	 * FIX: Fetch interface address list from driver. Do not include
	 * the P2P Device address if it is never used as interface address.
	 */
	/* P2P Interface Address Count */
	wpabuf_put_u8(buf, 1);
	wpabuf_put_data(buf, p2p->cfg->dev_addr, ETH_ALEN);
}


void p2p_buf_add_oob_go_neg_channel(struct wpabuf *buf, const char *country,
				    u8 oper_class, u8 channel,
				    enum p2p_role_indication role)
{
	/* OOB Group Owner Negotiation Channel */
	wpabuf_put_u8(buf, P2P_ATTR_OOB_GO_NEG_CHANNEL);
	wpabuf_put_le16(buf, 6);
	wpabuf_put_data(buf, country, 3);
	wpabuf_put_u8(buf, oper_class); /* Operating Class */
	wpabuf_put_u8(buf, channel); /* Channel Number */
	wpabuf_put_u8(buf, (u8) role); /* Role indication */
	wpa_printf(MSG_DEBUG, "P2P: * OOB GO Negotiation Channel: Operating "
		   "Class %u Channel %u Role %d",
		   oper_class, channel, role);
}


void p2p_buf_add_service_hash(struct wpabuf *buf, struct p2p_data *p2p)
{
	if (!p2p)
		return;

	/* Service Hash */
	wpabuf_put_u8(buf, P2P_ATTR_SERVICE_HASH);
	wpabuf_put_le16(buf, p2p->p2ps_seek_count * P2PS_HASH_LEN);
	wpabuf_put_data(buf, p2p->p2ps_seek_hash,
			p2p->p2ps_seek_count * P2PS_HASH_LEN);
	wpa_hexdump(MSG_DEBUG, "P2P: * Service Hash",
		    p2p->p2ps_seek_hash, p2p->p2ps_seek_count * P2PS_HASH_LEN);
}


void p2p_buf_add_session_info(struct wpabuf *buf, const char *info)
{
	size_t info_len = 0;

	if (info && info[0])
		info_len = os_strlen(info);

	/* Session Information Data Info */
	wpabuf_put_u8(buf, P2P_ATTR_SESSION_INFORMATION_DATA);
	wpabuf_put_le16(buf, (u16) info_len);

	if (info) {
		wpabuf_put_data(buf, info, info_len);
		wpa_printf(MSG_DEBUG, "P2P: * Session Info Data (%s)", info);
	}
}


void p2p_buf_add_connection_capability(struct wpabuf *buf, u8 connection_cap)
{
	/* Connection Capability Info */
	wpabuf_put_u8(buf, P2P_ATTR_CONNECTION_CAPABILITY);
	wpabuf_put_le16(buf, 1);
	wpabuf_put_u8(buf, connection_cap);
	wpa_printf(MSG_DEBUG, "P2P: * Connection Capability: 0x%x",
		   connection_cap);
}


void p2p_buf_add_advertisement_id(struct wpabuf *buf, u32 id, const u8 *mac)
{
	if (!buf || !mac)
		return;

	/* Advertisement ID Info */
	wpabuf_put_u8(buf, P2P_ATTR_ADVERTISEMENT_ID);
	wpabuf_put_le16(buf, (u16) (sizeof(u32) + ETH_ALEN));
	wpabuf_put_le32(buf, id);
	wpabuf_put_data(buf, mac, ETH_ALEN);
	wpa_printf(MSG_DEBUG, "P2P: * Advertisement ID (%x) " MACSTR,
		   id, MAC2STR(mac));
}


void p2p_buf_add_service_instance(struct wpabuf *buf, struct p2p_data *p2p,
				  u8 hash_count, const u8 *hash,
				  struct p2ps_advertisement *adv_list)
{
	struct p2ps_advertisement *adv;
	struct wpabuf *tmp_buf;
	const u8 *test;
	u8 count;
	int wsb_dev_wildcard;
	u8 *tag_len = NULL, *ie_len = NULL;
	size_t svc_len = 0, remaining = 0, total_len = 0;

	if (!adv_list || !hash || !hash_count)
		return;

	/* Allocate temp buffer, allowing for overflow of 1 instance */
	tmp_buf = wpabuf_alloc(MAX_SVC_ADV_IE_LEN + 256 + P2PS_HASH_LEN);
	if (!tmp_buf)
		return;

	for (count = hash_count, test = hash, wsb_dev_wildcard = 0;
	     count; count--) {
		/* Check for P2PS wildcard */
		if (os_memcmp(test, p2p->wild_card_hash, P2PS_HASH_LEN) == 0) {
			total_len = MAX_SVC_ADV_LEN + 1;
			goto wild_hash;
		}

		/* Check for WSB device wildcard */
		if (!wsb_dev_wildcard &&
		    os_memcmp(test, p2p->wildcard_hash_wsb_dev,
			      P2PS_HASH_LEN) == 0)
			wsb_dev_wildcard = 1;

		test += P2PS_HASH_LEN;
	}

	for (adv = adv_list; adv && total_len <= MAX_SVC_ADV_LEN;
	     adv = adv->next) {

		/* match a name hash or WSB device name prefix */
		for (count = hash_count, test = hash; count; count--) {
			if (os_memcmp(test, adv->hash, P2PS_HASH_LEN) == 0 ||
			    (wsb_dev_wildcard &&
			     os_memcmp(P2PS_WILD_HASH_WSB_DEV_STR,
				       adv->svc_name,
				       sizeof(P2PS_WILD_HASH_WSB_DEV_STR) - 1)
			     == 0))
				goto hash_match;

			test += P2PS_HASH_LEN;
		}

		/* No matches found - Skip this Adv Instance */
		continue;

hash_match:
		if (!tag_len) {
			tag_len = p2p_buf_add_ie_hdr(tmp_buf);
			remaining = 255 - 4;
			if (!ie_len) {
				wpabuf_put_u8(tmp_buf,
					      P2P_ATTR_ADVERTISED_SERVICE);
				ie_len = wpabuf_put(tmp_buf, sizeof(u16));
				remaining -= (sizeof(u8) + sizeof(u16));
			}
		}

		svc_len = os_strlen(adv->svc_name);

		if (7 + svc_len + total_len > MAX_SVC_ADV_LEN) {
			/* Can't fit... return wildcard */
			total_len = MAX_SVC_ADV_LEN + 1;
			break;
		}

		if (remaining <= (sizeof(adv->id) +
				  sizeof(adv->config_methods))) {
			size_t front = remaining;
			size_t back = (sizeof(adv->id) +
				       sizeof(adv->config_methods)) - front;
			u8 holder[sizeof(adv->id) +
				  sizeof(adv->config_methods)];

			/* This works even if front or back == 0 */
			WPA_PUT_LE32(holder, adv->id);
			WPA_PUT_BE16(&holder[sizeof(adv->id)],
				     adv->config_methods);
			wpabuf_put_data(tmp_buf, holder, front);
			p2p_buf_update_ie_hdr(tmp_buf, tag_len);
			tag_len = p2p_buf_add_ie_hdr(tmp_buf);
			wpabuf_put_data(tmp_buf, &holder[front], back);
			remaining = 255 - (sizeof(adv->id) +
					   sizeof(adv->config_methods)) - back;
		} else {
			wpabuf_put_le32(tmp_buf, adv->id);
			wpabuf_put_be16(tmp_buf, adv->config_methods);
			remaining -= (sizeof(adv->id) +
				      sizeof(adv->config_methods));
		}

		/* We are guaranteed at least one byte for svc_len */
		wpabuf_put_u8(tmp_buf, svc_len);
		remaining -= sizeof(u8);

		if (remaining < svc_len) {
			size_t front = remaining;
			size_t back = svc_len - front;

			wpabuf_put_data(tmp_buf, adv->svc_name, front);
			p2p_buf_update_ie_hdr(tmp_buf, tag_len);
			tag_len = p2p_buf_add_ie_hdr(tmp_buf);

			/* In rare cases, we must split across 3 attributes */
			if (back > 255 - 4) {
				wpabuf_put_data(tmp_buf,
						&adv->svc_name[front], 255 - 4);
				back -= 255 - 4;
				front += 255 - 4;
				p2p_buf_update_ie_hdr(tmp_buf, tag_len);
				tag_len = p2p_buf_add_ie_hdr(tmp_buf);
			}

			wpabuf_put_data(tmp_buf, &adv->svc_name[front], back);
			remaining = 255 - 4 - back;
		} else {
			wpabuf_put_data(tmp_buf, adv->svc_name, svc_len);
			remaining -= svc_len;
		}

		/*           adv_id      config_methods     svc_string */
		total_len += sizeof(u32) + sizeof(u16) + sizeof(u8) + svc_len;
	}

	if (tag_len)
		p2p_buf_update_ie_hdr(tmp_buf, tag_len);

	if (ie_len)
		WPA_PUT_LE16(ie_len, (u16) total_len);

wild_hash:
	/* If all fit, return matching instances, otherwise the wildcard */
	if (total_len <= MAX_SVC_ADV_LEN) {
		wpabuf_put_buf(buf, tmp_buf);
	} else {
		char *wild_card = P2PS_WILD_HASH_STR;
		u8 wild_len;

		/* Insert wildcard instance */
		tag_len = p2p_buf_add_ie_hdr(buf);
		wpabuf_put_u8(buf, P2P_ATTR_ADVERTISED_SERVICE);
		ie_len = wpabuf_put(buf, sizeof(u16));

		wild_len = (u8) os_strlen(wild_card);
		wpabuf_put_le32(buf, 0);
		wpabuf_put_be16(buf, 0);
		wpabuf_put_u8(buf, wild_len);
		wpabuf_put_data(buf, wild_card, wild_len);

		WPA_PUT_LE16(ie_len, 4 + 2 + 1 + wild_len);
		p2p_buf_update_ie_hdr(buf, tag_len);
	}

	wpabuf_free(tmp_buf);
}


void p2p_buf_add_session_id(struct wpabuf *buf, u32 id, const u8 *mac)
{
	if (!buf || !mac)
		return;

	/* Session ID Info */
	wpabuf_put_u8(buf, P2P_ATTR_SESSION_ID);
	wpabuf_put_le16(buf, (u16) (sizeof(u32) + ETH_ALEN));
	wpabuf_put_le32(buf, id);
	wpabuf_put_data(buf, mac, ETH_ALEN);
	wpa_printf(MSG_DEBUG, "P2P: * Session ID Info (%x) " MACSTR,
		   id, MAC2STR(mac));
}


void p2p_buf_add_feature_capability(struct wpabuf *buf, u16 len, const u8 *mask)
{
	if (!buf || !len || !mask)
		return;

	/* Feature Capability */
	wpabuf_put_u8(buf, P2P_ATTR_FEATURE_CAPABILITY);
	wpabuf_put_le16(buf, len);
	wpabuf_put_data(buf, mask, len);
	wpa_printf(MSG_DEBUG, "P2P: * Feature Capability (%d)", len);
}


void p2p_buf_add_persistent_group_info(struct wpabuf *buf, const u8 *dev_addr,
				       const u8 *ssid, size_t ssid_len)
{
	/* P2P Group ID */
	wpabuf_put_u8(buf, P2P_ATTR_PERSISTENT_GROUP);
	wpabuf_put_le16(buf, ETH_ALEN + ssid_len);
	wpabuf_put_data(buf, dev_addr, ETH_ALEN);
	wpabuf_put_data(buf, ssid, ssid_len);
	wpa_printf(MSG_DEBUG, "P2P: * P2P Group ID " MACSTR,
		   MAC2STR(dev_addr));
}


static int p2p_add_wps_string(struct wpabuf *buf, enum wps_attribute attr,
			      const char *val)
{
	size_t len;

	len = val ? os_strlen(val) : 0;
	if (wpabuf_tailroom(buf) < 4 + len)
		return -1;
	wpabuf_put_be16(buf, attr);
#ifndef CONFIG_WPS_STRICT
	if (len == 0) {
		/*
		 * Some deployed WPS implementations fail to parse zeor-length
		 * attributes. As a workaround, send a space character if the
		 * device attribute string is empty.
		 */
		if (wpabuf_tailroom(buf) < 3)
			return -1;
		wpabuf_put_be16(buf, 1);
		wpabuf_put_u8(buf, ' ');
		return 0;
	}
#endif /* CONFIG_WPS_STRICT */
	wpabuf_put_be16(buf, len);
	if (val)
		wpabuf_put_data(buf, val, len);
	return 0;
}


int p2p_build_wps_ie(struct p2p_data *p2p, struct wpabuf *buf, int pw_id,
		     int all_attr)
{
	u8 *len;
	int i;

	if (wpabuf_tailroom(buf) < 6)
		return -1;
	wpabuf_put_u8(buf, WLAN_EID_VENDOR_SPECIFIC);
	len = wpabuf_put(buf, 1);
	wpabuf_put_be32(buf, WPS_DEV_OUI_WFA);

	if (wps_build_version(buf) < 0)
		return -1;

	if (all_attr) {
		if (wpabuf_tailroom(buf) < 5)
			return -1;
		wpabuf_put_be16(buf, ATTR_WPS_STATE);
		wpabuf_put_be16(buf, 1);
		wpabuf_put_u8(buf, WPS_STATE_NOT_CONFIGURED);
	}

	if (pw_id >= 0) {
		if (wpabuf_tailroom(buf) < 6)
			return -1;
		/* Device Password ID */
		wpabuf_put_be16(buf, ATTR_DEV_PASSWORD_ID);
		wpabuf_put_be16(buf, 2);
		wpa_printf(MSG_DEBUG, "P2P: WPS IE Device Password ID: %d",
			   pw_id);
		wpabuf_put_be16(buf, pw_id);
	}

	if (all_attr) {
		if (wpabuf_tailroom(buf) < 5)
			return -1;
		wpabuf_put_be16(buf, ATTR_RESPONSE_TYPE);
		wpabuf_put_be16(buf, 1);
		wpabuf_put_u8(buf, WPS_RESP_ENROLLEE_INFO);

		if (wps_build_uuid_e(buf, p2p->cfg->uuid) < 0 ||
		    p2p_add_wps_string(buf, ATTR_MANUFACTURER,
				       p2p->cfg->manufacturer) < 0 ||
		    p2p_add_wps_string(buf, ATTR_MODEL_NAME,
				       p2p->cfg->model_name) < 0 ||
		    p2p_add_wps_string(buf, ATTR_MODEL_NUMBER,
				       p2p->cfg->model_number) < 0 ||
		    p2p_add_wps_string(buf, ATTR_SERIAL_NUMBER,
				       p2p->cfg->serial_number) < 0)
			return -1;

		if (wpabuf_tailroom(buf) < 4 + WPS_DEV_TYPE_LEN)
			return -1;
		wpabuf_put_be16(buf, ATTR_PRIMARY_DEV_TYPE);
		wpabuf_put_be16(buf, WPS_DEV_TYPE_LEN);
		wpabuf_put_data(buf, p2p->cfg->pri_dev_type, WPS_DEV_TYPE_LEN);

		if (p2p_add_wps_string(buf, ATTR_DEV_NAME, p2p->cfg->dev_name)
		    < 0)
			return -1;

		if (wpabuf_tailroom(buf) < 6)
			return -1;
		wpabuf_put_be16(buf, ATTR_CONFIG_METHODS);
		wpabuf_put_be16(buf, 2);
		wpabuf_put_be16(buf, p2p->cfg->config_methods);
	}

	if (wps_build_wfa_ext(buf, 0, NULL, 0) < 0)
		return -1;

	if (all_attr && p2p->cfg->num_sec_dev_types) {
		if (wpabuf_tailroom(buf) <
		    4 + WPS_DEV_TYPE_LEN * p2p->cfg->num_sec_dev_types)
			return -1;
		wpabuf_put_be16(buf, ATTR_SECONDARY_DEV_TYPE_LIST);
		wpabuf_put_be16(buf, WPS_DEV_TYPE_LEN *
				p2p->cfg->num_sec_dev_types);
		wpabuf_put_data(buf, p2p->cfg->sec_dev_type,
				WPS_DEV_TYPE_LEN *
				p2p->cfg->num_sec_dev_types);
	}

	/* Add the WPS vendor extensions */
	for (i = 0; i < P2P_MAX_WPS_VENDOR_EXT; i++) {
		if (p2p->wps_vendor_ext[i] == NULL)
			break;
		if (wpabuf_tailroom(buf) <
		    4 + wpabuf_len(p2p->wps_vendor_ext[i]))
			continue;
		wpabuf_put_be16(buf, ATTR_VENDOR_EXT);
		wpabuf_put_be16(buf, wpabuf_len(p2p->wps_vendor_ext[i]));
		wpabuf_put_buf(buf, p2p->wps_vendor_ext[i]);
	}

	p2p_buf_update_ie_hdr(buf, len);

	return 0;
}
