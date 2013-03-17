//
//  settings.c
//
//  Created by Ubaka Onyechi on 19/02/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#import "settings.h"
#import "earth.h"
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SETTINGS_SAVE_FILE "settings.dat"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *s_list [3] = 
{
  "Edit Cities",
  "Temperature",
  "Show Time",
};

static const char *s_tempUnitStr [2] = 
{
  "Celsius",
  "Farenheit",
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
  const char **cityNames;
  bool        *cityDisplay;
  int          cityCount;
  int          tempUnit;
  bool         showTime;
} settings_t;

static settings_t s_settings;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CityTableViewController : UITableViewController
{
}
@end

@implementation CityTableViewController

- (id) initWithStyle:(UITableViewStyle)style
{
  self = [super initWithStyle:style];
  
  if (self)
  {
  }
  
  return self;
}

- (void) dealloc
{
  [super dealloc];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  if (section == 0)
  {
    return s_settings.cityCount;
  }

  return 0;
}

- (void)viewWillDisappear:(BOOL)animated
{
  [[self navigationController] popToRootViewControllerAnimated:FALSE];
}

- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  NSString *identifier = @"CityCell";
  
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
  
  if (cell == nil) 
  {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
  }
  
  int idx = indexPath.row;
  
  CX_ASSERT (idx < s_settings.cityCount);
  CX_ASSERT (s_settings.cityNames);
  
  const char *label = s_settings.cityNames [idx];
  bool display = s_settings.cityDisplay [idx];
  
  cell.textLabel.text = [NSString stringWithCString:label encoding:NSASCIIStringEncoding];  
  cell.selectionStyle = UITableViewCellSelectionStyleNone;
  cell.accessoryType = display ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
  
  return cell;
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
  int idx = indexPath.row;
  UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
  
  CX_ASSERT (cell);
  CX_ASSERT (idx < s_settings.cityCount);
  
  bool display = !s_settings.cityDisplay [idx];
  s_settings.cityDisplay [idx] = display;
  
  cell.accessoryType = display ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface TemperatureTableViewController : UITableViewController
{
}
@end

@implementation TemperatureTableViewController

- (id) initWithStyle:(UITableViewStyle)style
{
  self = [super initWithStyle:style];
  
  if (self)
  {
  }
  
  return self;
}

- (void) dealloc
{
  [super dealloc];
}

- (void)viewWillDisappear:(BOOL)animated
{
  [[self navigationController] popToRootViewControllerAnimated:FALSE];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
  int oldUnit = s_settings.tempUnit;
  int newUnit = indexPath.row;
  
  if (newUnit != oldUnit)
  {
    UITableViewCell *newCell = [tableView cellForRowAtIndexPath:indexPath];
    UITableViewCell *oldCell = [tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:oldUnit inSection:0]];
    
    CX_ASSERT (newCell);
    CX_ASSERT (oldCell);
    
    newCell.accessoryType = UITableViewCellAccessoryCheckmark;
    oldCell.accessoryType = UITableViewCellAccessoryNone;
    
    s_settings.tempUnit = newUnit;
  }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  if (section == 0)
  {
    return 2;
  }
  
  return 0;
}

- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  NSString *identifier = @"TempCell";
  
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
  
  if (cell == nil) 
  {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
  }
  
  int idx = indexPath.row;
  
  cell.selectionStyle = UITableViewCellSelectionStyleNone;
  cell.accessoryType = (s_settings.tempUnit == indexPath.row) ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
  cell.textLabel.text = [NSString stringWithCString:s_tempUnitStr [idx] encoding:NSASCIIStringEncoding];

  return cell;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface RootTableViewController : UITableViewController
{
  UINavigationController *_navCtrlr;
  UISwitch *_timeSwitch;
  TemperatureTableViewController *_temperatureViewController;
  CityTableViewController *_cityViewController;
}
@end


@implementation RootTableViewController

- (id) initWithStyle:(UITableViewStyle)style
{
  self = [super initWithStyle:style];
  
  if (self)
  {
    _timeSwitch = [[UISwitch alloc] init];
    _temperatureViewController = [[TemperatureTableViewController alloc] initWithStyle:UITableViewStylePlain];
    _cityViewController = [[CityTableViewController alloc] initWithStyle:UITableViewStylePlain];
    _navCtrlr = [[UINavigationController alloc] initWithRootViewController:self];
  }
  
  return self;
}

- (void) dealloc
{
  [_cityViewController release];
  [_temperatureViewController release];
  [_timeSwitch release];
  [_navCtrlr release];
  
  [super dealloc];
}

- (void)switchTouched
{
  bool showTime = !s_settings.showTime;
  s_settings.showTime = showTime;
  [_timeSwitch setOn:showTime animated:YES];
}

- (void)viewDidLoad
{
  CX_DEBUGLOG_CONSOLE (1, "RootTableViewController: viewDidLoad");
  
  [_timeSwitch addTarget:self action:@selector(switchTouched) forControlEvents:UIControlEventTouchUpInside];
}

- (void)viewDidUnload
{
  [_timeSwitch removeTarget:self action:@selector(switchTouched) forControlEvents:UIControlEventTouchUpInside];
}

- (void)viewWillAppear:(BOOL)animated
{
  CX_DEBUGLOG_CONSOLE (1, "RootTableViewController: viewWillAppear");
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
  switch (indexPath.row) 
  {
    case 0: // edit cities
    {
      [_navCtrlr pushViewController:_cityViewController animated:YES];
      
      break;
    }
      
    case 1: // temperature unit
    {
      [_navCtrlr pushViewController:_temperatureViewController animated:YES];
      
      break;
    }
      
    case 2: // show time
    {
      break;
    }
      
    default:
      break;
  }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  CX_DEBUGLOG_CONSOLE (1, "RootTableViewController: tableView numberOfRowsInSection:");
  
  if (section == 0)
  {
    return 3;
  }
  
  return 0;
}

- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  CX_DEBUGLOG_CONSOLE (1, "RootTableViewController: tableView cellForRowAtIndexPath:");
  
  NSString *identifier = @"RootCell";
  
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
  
  if (cell == nil) 
  {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
  }
    
  int idx = indexPath.row;
  
  switch (idx)
  {
    case 0: // edit cities
    {
      cell.selectionStyle = UITableViewCellSelectionStyleNone;
      cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
      break;
    }
      
    case 1: // temperature unit
    {
      cell.selectionStyle = UITableViewCellSelectionStyleNone;
      cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
      cell.detailTextLabel.text = [NSString stringWithCString:s_tempUnitStr [s_settings.tempUnit] encoding:NSASCIIStringEncoding];
      
      break;
    }
      
    case 2: // show time
    {
      [_timeSwitch setOn:s_settings.showTime];
      
      cell.accessoryView = _timeSwitch;
      cell.selectionStyle = UITableViewCellSelectionStyleNone;
      cell.accessoryType = UITableViewCellAccessoryNone;
      break;
    }
      
    default:
    {
      CX_FATAL_ERROR ("invalid row");
      break;
    }
  }
  
  cell.textLabel.text = [NSString stringWithCString:s_list [idx] encoding:NSASCIIStringEncoding];

#if 0
  cell.contentView.backgroundColor = [UIColor clearColor];
  cell.backgroundColor = [UIColor clearColor];
  UIView *bgView = [[UIView alloc] init];
  [[bgView layer] setCornerRadius:10.0f];
  [bgView setBackgroundColor:[UIColor colorWithWhite:1.0f alpha:0.25f]];
  cell.backgroundView = bgView;
#endif
  
  return cell;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool s_initialised = false;
static bool s_uiActive = false;
static UIViewController *s_rootViewCtrlr = nil;
static UIPopoverController *s_popover = nil;
static RootTableViewController *s_rootTableViewCtrlr = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void settings_init_view_create (void);
static void settings_init_view_destroy (void);
static bool settings_save (const char *filename);
static bool settings_load (const char *filename, cx_file_storage_base base);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool settings_init (void *rootvc, const char *filename)
{
  CX_ASSERT (!s_initialised);
  CX_ASSERT (rootvc);
  
  bool init = false;
  
  memset (&s_settings, 0, sizeof (s_settings));
  
  s_rootViewCtrlr = (UIViewController *) rootvc;
  
  settings_init_view_create ();
    
  if (settings_load (SETTINGS_SAVE_FILE, CX_FILE_STORAGE_BASE_DOCUMENTS))
  {
    init = true;
  }
  else if (settings_load (filename, CX_FILE_STORAGE_BASE_RESOURCE))
  {
    init = settings_save (SETTINGS_SAVE_FILE);
  }
  
  CX_DEBUGLOG_CONSOLE (1 && !init, "settings_init: failed");
  
  s_initialised = init;
  
  return s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_deinit (void)
{
  CX_ASSERT (s_initialised);
  
  settings_init_view_destroy ();
  
  cx_free (s_settings.cityDisplay);
  
  s_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_ui_show (void)
{
  UIView *parentView = s_rootViewCtrlr.view;
  
  float viewPosX = parentView.bounds.origin.x;
  float viewPosY = parentView.bounds.origin.y;
  float viewWidth  = parentView.bounds.size.width;
  float viewHeight = parentView.bounds.size.height;
  
  float width = 800.0f;
  float height = 600.0f;
  float posX = viewPosX + ((viewWidth - width) * 0.5f);
  float posY = viewPosY + ((viewHeight - height) * 0.5f);
  
  [s_popover setPassthroughViews:[NSArray arrayWithObject:parentView]];
  [s_popover presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
  
  s_uiActive = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_ui_hide (void)
{
  [s_popover dismissPopoverAnimated:YES];
  
  s_uiActive = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool settings_ui_active (void)
{
  return s_uiActive;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_set_city_names (const char **names, int count)
{
  CX_ASSERT (s_settings.cityCount == count);
  
  s_settings.cityNames = names;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool settings_get_city_display (int cityIdx)
{
  bool b = s_settings.cityDisplay [cityIdx];
  
  return b;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool settings_get_show_clock (void)
{
  return s_settings.showTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int settings_get_temperature_unit (void)
{
  return s_settings.tempUnit;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void settings_init_view_create (void)
{ 
  s_rootTableViewCtrlr = [[RootTableViewController alloc] initWithStyle:UITableViewStyleGrouped];
  
  s_popover = [[UIPopoverController alloc] initWithContentViewController:[s_rootTableViewCtrlr navigationController]];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void settings_init_view_destroy (void)
{
  [s_rootTableViewCtrlr release];
  
  [s_popover release];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool settings_save (const char *filename)
{
/*
  {
      "settings": {
      "time": true,
      "temp": 0,
      "city": [
               0,
               0,
               0]
    }
  }
*/
  
  cxu8 buffer [2048];
  cxu32 bufferlen = 0;
  
  buffer [0] = 0;
  
  char tmp [1024];
  
  int l = cx_sprintf (tmp, 1024, "{\"settings\":{\"time\":%d,\"temp\":%d,\"city\":[", s_settings.showTime ? 1 : 0, s_settings.tempUnit);
  
  CX_ASSERT (l >= 0);
  
  char *buf = cx_strcat ((char *) buffer, 2048, tmp);
  
  bufferlen += l;
  
  for (int i = 0, c = s_settings.cityCount; i < c; ++i)
  {
    int b = s_settings.cityDisplay [i] ? 1 : 0;
    
    if (i != 0)
    {
      l = cx_sprintf (tmp, 1024, ",%d", b);
      CX_ASSERT (l >= 0);
    }
    else
    {
      l = cx_sprintf (tmp, 1024, "%d", b);
      CX_ASSERT (l >= 0);
    }
    
    buf = cx_strcat (buf, 2048 - bufferlen, tmp);
    
    bufferlen += l;
  }
  
  cx_strcat (buf, 2048 - bufferlen, "]}}");
  
  bufferlen += 3;
  
  CX_ASSERT (bufferlen < 2048);
  
  buffer [bufferlen] = 0;
  
  bool success = cx_file_storage_save_contents (buffer, bufferlen, filename, CX_FILE_STORAGE_BASE_DOCUMENTS);

  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool settings_load (const char *filename, cx_file_storage_base base)
{
  bool success = false;
  
  cxu8 *data = NULL;
  cxu32 datasize;
  
  if (cx_file_storage_load_contents (&data, &datasize, filename, base))
  {
    cx_json_tree jsonTree = cx_json_tree_create ((const char *) data, datasize);
    
    if (jsonTree)
    {
      cx_json_node rootNode = cx_json_tree_root_node (jsonTree);
      cx_json_node settingsNode = cx_json_object_child (rootNode, "settings");
      
      cx_json_node timeNode = cx_json_object_child (settingsNode, "time");
      cx_json_node tempNode = cx_json_object_child (settingsNode, "temp");
      cx_json_node cityNode = cx_json_object_child (settingsNode, "city");
      
      CX_ASSERT (timeNode);
      CX_ASSERT (tempNode);
      CX_ASSERT (cityNode);
      
      int showTime = (int) cx_json_value_int (timeNode);
      int tempUnit = (int) cx_json_value_int (tempNode);
      int cityCount = cx_json_array_size (cityNode);
      bool *cityDisplay = cx_malloc (sizeof (bool) * cityCount);
      
      for (int i = 0; i < cityCount; ++i)
      {
        cx_json_node m = cx_json_array_member (cityNode, i);
        
        bool on = cx_json_value_int (m) ? true : false;
        
        cityDisplay [i] = on;
      }
        
      s_settings.showTime = showTime ? true : false;
      s_settings.tempUnit = tempUnit;
      s_settings.cityCount = cityCount;
      s_settings.cityDisplay = cityDisplay;
      
      cx_json_tree_destroy (jsonTree);
      
      success = true;
    }
    else
    {
      CX_DEBUGLOG_CONSOLE (1, "JSON tree parse error: %s", filename);
    }
    
    cx_free (data);
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, "Failed to load %s", filename);
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

