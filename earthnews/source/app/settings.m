//
//  settings.c
//
//  Created by Ubaka Onyechi on 19/02/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#import "settings.h"
#import "util.h"
#import "earth.h"
#import <UIKit/UIKit.h>
//#import <QuartzCore/QuartzCore.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SETTINGS_SAVE_FILE              "settings.dat"
#define SETTINGS_ROOT_TABLE_DATA_ROWS   (4)
#define SETTINGS_TEMP_TABLE_DATA_ROWS   (2)
#define SETTINGS_CLOCK_TABLE_DATA_ROWS  (2)
#define SCREEN_FADE_OPACITY             (0.6f)
#define SCREEN_FADE_DURATION            (0.5f)
#define SETTINGS_UI_SIZE_WIDTH          (320.0f)
#define SETTINGS_UI_SIZE_HEIGHT         (400.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *s_rootTableData [SETTINGS_ROOT_TABLE_DATA_ROWS] = 
{
  "Cities",
  "Temperature",
  "Clock Format",
  "Show Time",
};

static const char *s_tempTableData [SETTINGS_TEMP_TABLE_DATA_ROWS] = 
{
  "Celsius",
  "Farenheit",
};

static const char *s_clockTableData [SETTINGS_TEMP_TABLE_DATA_ROWS] = 
{
  "12-hour",
  "24-hour",
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
  int          clockFmt;
  bool         showTime;
} settings_t;

static settings_t s_settings;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CityTableViewController : UITableViewController
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface TemperatureTableViewController : UITableViewController
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface ClockTableViewController : UITableViewController
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface RootTableViewController : UITableViewController
{
  UINavigationController *_navCtrlr;
  UISwitch *_timeSwitch;
  UIBarButtonItem *_doneButton;
  TemperatureTableViewController *_temperatureViewController;
  CityTableViewController *_cityViewController;
  ClockTableViewController *_clockViewController;
}
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface PopoverBackground : UIPopoverBackgroundView
@property (nonatomic, readwrite) UIPopoverArrowDirection arrowDirection;
@property (nonatomic, readwrite) CGFloat arrowOffset;
@end

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

bool settings_init (const void *rootvc, const char *filename)
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
  
  s_rootViewCtrlr = nil;
  
  s_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_data_save (void)
{
  bool saved = settings_save (SETTINGS_SAVE_FILE);
  
  CX_ASSERT (saved);
  
  CX_REF_UNUSED (saved);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_ui_show (void)
{
  if (!s_uiActive)
  {
    UIView *parentView = s_rootViewCtrlr.view;

    float viewPosX = parentView.bounds.origin.x;
    float viewPosY = parentView.bounds.origin.y;
    float viewWidth  = parentView.bounds.size.width;
    float viewHeight = parentView.bounds.size.height;
    float width = SETTINGS_UI_SIZE_WIDTH;
    float height = SETTINGS_UI_SIZE_HEIGHT;
    float posX = viewPosX + ((viewWidth - width) * 0.5f);
    float posY = viewPosY + ((viewHeight - height) * 0.5f);
    
    //[s_popover setPopoverContentSize:CGSizeMake(width, height)];
    [s_popover setPassthroughViews:[NSArray arrayWithObject:parentView]];
    [s_popover presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
    
    util_screen_fade_trigger (SCREEN_FADE_TYPE_OUT, SCREEN_FADE_OPACITY, SCREEN_FADE_DURATION, NULL, NULL);
    
    s_uiActive = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void settings_ui_hide (void)
{
  if (s_uiActive)
  {
    [s_popover dismissPopoverAnimated:YES];
    
    util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, SCREEN_FADE_OPACITY, SCREEN_FADE_DURATION, NULL, NULL);
    
    s_uiActive = false;
  }
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

int settings_get_clock_display_format (void)
{
  return s_settings.clockFmt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void settings_init_view_create (void)
{  
  s_rootTableViewCtrlr = [[RootTableViewController alloc] initWithStyle:UITableViewStyleGrouped];
  
  s_popover = [[UIPopoverController alloc] initWithContentViewController:[s_rootTableViewCtrlr navigationController]];
  
  [s_popover setPopoverBackgroundViewClass:[PopoverBackground class]];
  
  [s_rootTableViewCtrlr setTitle:@"Settings"];
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
  cxu8 buffer [2048];
  cxu32 bufferlen = 0;
  
  buffer [0] = 0;
  
  char tmp [1024];
  
  int l = cx_sprintf (tmp, 1024, "{\"settings\":{\"time\":%d,\"cloc\":%d,\"temp\":%d,\"city\":[", 
                      s_settings.showTime ? 1 : 0, 
                      s_settings.clockFmt,
                      s_settings.tempUnit);
  
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
      cx_json_node clocNode = cx_json_object_child (settingsNode, "cloc");
      
      CX_ASSERT (timeNode);
      CX_ASSERT (tempNode);
      CX_ASSERT (cityNode);
      CX_ASSERT (clocNode);
      
      int showTime = (int) cx_json_value_int (timeNode);
      int tempUnit = (int) cx_json_value_int (tempNode);
      int clockFmt = (int) cx_json_value_int (clocNode);
      int cityCount = cx_json_array_size (cityNode);
      bool *cityDisplay = cx_malloc (sizeof (bool) * cityCount);
      
      for (int i = 0; i < cityCount; ++i)
      {
        cx_json_node m = cx_json_array_member (cityNode, i);
        
        bool on = cx_json_value_int (m) ? true : false;
        
        cityDisplay [i] = on;
      }
      if (s_settings.cityDisplay)
      {
        cx_free (s_settings.cityDisplay);
        s_settings.cityDisplay = NULL;
      }
        
      s_settings.showTime = showTime ? true : false;
      s_settings.tempUnit = tempUnit;
      s_settings.clockFmt = clockFmt;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation CityTableViewController

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  return (section == 0) ? s_settings.cityCount : 0;
}

- (void)viewWillAppear:(BOOL)animated
{
  [self setContentSizeForViewInPopover:CGSizeMake (SETTINGS_UI_SIZE_WIDTH, SETTINGS_UI_SIZE_HEIGHT)];
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
  
  cell.textLabel.text = [NSString stringWithCString:label encoding:NSUTF8StringEncoding];
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

@implementation TemperatureTableViewController

- (void)viewWillDisappear:(BOOL)animated
{
  [[self navigationController] popToRootViewControllerAnimated:FALSE];
}

- (void)viewWillAppear:(BOOL)animated
{
  [self setContentSizeForViewInPopover:CGSizeMake (SETTINGS_UI_SIZE_WIDTH, SETTINGS_UI_SIZE_HEIGHT)];
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
  return (section == 0) ? SETTINGS_TEMP_TABLE_DATA_ROWS : 0;
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
  cell.textLabel.text = [NSString stringWithCString:s_tempTableData [idx] encoding:NSUTF8StringEncoding];
  
  return cell;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation ClockTableViewController

- (void)viewWillDisappear:(BOOL)animated
{
  [[self navigationController] popToRootViewControllerAnimated:FALSE];
}

- (void)viewWillAppear:(BOOL)animated
{
  [self setContentSizeForViewInPopover:CGSizeMake (SETTINGS_UI_SIZE_WIDTH, SETTINGS_UI_SIZE_HEIGHT)];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
  int oldUnit = s_settings.clockFmt;
  int newUnit = indexPath.row;
  
  if (newUnit != oldUnit)
  {
    UITableViewCell *newCell = [tableView cellForRowAtIndexPath:indexPath];
    UITableViewCell *oldCell = [tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:oldUnit inSection:0]];
    
    CX_ASSERT (newCell);
    CX_ASSERT (oldCell);
    
    newCell.accessoryType = UITableViewCellAccessoryCheckmark;
    oldCell.accessoryType = UITableViewCellAccessoryNone;
    
    s_settings.clockFmt = newUnit;
  }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  return (section == 0) ? SETTINGS_CLOCK_TABLE_DATA_ROWS : 0;
}

- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  NSString *identifier = @"ClockCell";
  
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
  
  if (cell == nil)
  {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
  }
  
  int idx = indexPath.row;
  
  cell.selectionStyle = UITableViewCellSelectionStyleNone;
  cell.accessoryType = (s_settings.clockFmt == indexPath.row) ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
  cell.textLabel.text = [NSString stringWithCString:s_clockTableData [idx] encoding:NSUTF8StringEncoding];
  
  return cell;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation RootTableViewController

- (id)initWithStyle:(UITableViewStyle)style
{
  self = [super initWithStyle:style];
  
  if (self)
  {
    _timeSwitch = [[UISwitch alloc] init];
    _temperatureViewController = [[TemperatureTableViewController alloc] initWithStyle:UITableViewStylePlain];
    _cityViewController = [[CityTableViewController alloc] initWithStyle:UITableViewStylePlain];
    _clockViewController = [[ClockTableViewController alloc] initWithStyle:UITableViewStylePlain];
    _navCtrlr = [[UINavigationController alloc] initWithRootViewController:self];
    
    _doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                                target:self
                                                                action:@selector(doneButtonClicked:)];
    
    //self.navigationItem.rightBarButtonItem = _doneButton;
    [self setContentSizeForViewInPopover:CGSizeMake (SETTINGS_UI_SIZE_WIDTH, SETTINGS_UI_SIZE_HEIGHT)];
    
    _navCtrlr.navigationBar.tintColor = [UIColor blackColor];
  }
  
  return self;
}

- (void)dealloc
{
  [_cityViewController release];
  [_temperatureViewController release];
  [_clockViewController release];
  [_timeSwitch release];
  [_doneButton release];
  [_navCtrlr release];
  
  [super dealloc];
}

- (void)switchTouched
{
  bool showTime = !s_settings.showTime;
  s_settings.showTime = showTime;
  [_timeSwitch setOn:showTime animated:YES];
}

- (void)doneButtonClicked:(id)sender
{
  settings_ui_hide ();
}

- (void)viewDidLoad
{
  [super viewDidLoad];
  [_timeSwitch addTarget:self action:@selector(switchTouched) forControlEvents:UIControlEventTouchUpInside];
}

- (void)viewDidUnload
{
  [super viewDidUnload];
  [_timeSwitch removeTarget:self action:@selector(switchTouched) forControlEvents:UIControlEventTouchUpInside];
}

- (void)viewWillAppear:(BOOL)animated
{
  [self setContentSizeForViewInPopover:CGSizeMake (SETTINGS_UI_SIZE_WIDTH, SETTINGS_UI_SIZE_HEIGHT)];
}

- (NSInteger) supportedInterfaceOrientations
{
  return UIInterfaceOrientationMaskLandscape;
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
      
    case 2: // clock
    {
      [_navCtrlr pushViewController:_clockViewController animated:YES];
      break;
    }
      
    case 3: // show time
    {
      break;
    }
      
    default:
      break;
  }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  return (section == 0) ? SETTINGS_ROOT_TABLE_DATA_ROWS : 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  NSString *identifier = @"RootCell";
  
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
  
  if (cell == nil)
  {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
  }
  
  int idx = indexPath.row;
  
  switch (idx)
  {
    case 0: // cities
    {
      cell.selectionStyle = UITableViewCellSelectionStyleNone;
      cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
      break;
    }
      
    case 1: // temperature unit
    {
      cell.selectionStyle = UITableViewCellSelectionStyleNone;
      cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
      cell.detailTextLabel.text = [NSString stringWithCString:s_tempTableData [s_settings.tempUnit] encoding:NSUTF8StringEncoding];
      break;
    }
      
    case 2: // clock
    {
      cell.selectionStyle = UITableViewCellSelectionStyleNone;
      cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
      cell.detailTextLabel.text = [NSString stringWithCString:s_clockTableData [s_settings.clockFmt] encoding:NSUTF8StringEncoding];
      break;
    }
      
    case 3: // show time
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
  
  cell.textLabel.text = [NSString stringWithCString:s_rootTableData [idx] encoding:NSUTF8StringEncoding];
  
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

@implementation PopoverBackground

@synthesize arrowOffset;
@synthesize arrowDirection;

- (id)initWithFrame:(CGRect)frame
{
  if (self = [super initWithFrame:frame])
  {
    self.backgroundColor = [UIColor colorWithWhite:0.2f alpha:0.7f];
    self.arrowDirection = 0;
    self.arrowOffset = 0.0f;
  }
  return self;
}

+ (UIEdgeInsets)contentViewInsets
{
  return UIEdgeInsetsMake (10.0f, 2.0f, 1.0f, 2.0f);
}

+ (CGFloat)arrowHeight
{
  return 0.0f;
}

+ (CGFloat)arrowBase
{
  return 0.0f;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
