module.exports = [
  {
    "type": "heading",
    "defaultValue": "Mighty Mouse Watch Face Settings"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Trackers"
      },
      {
        "type": "toggle",
        "messageKey": "KEY_SHOW_HEART_RATE",
        "label": "Show Heart Rate",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "KEY_SHOW_MIGHTY_MOUSE",
        "label": "Show Mighty Mouse",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "KEY_SHOW_STEPS",
        "label": "Show Steps",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "KEY_SHOW_BATTERY",
        "label": "Show Battery Level",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];