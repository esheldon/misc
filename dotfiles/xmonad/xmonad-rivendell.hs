-- <+> is a "combinator" defined in one of the above modules
import XMonad
import XMonad.Hooks.DynamicLog
import XMonad.Hooks.ManageDocks
import XMonad.Util.Run(spawnPipe)
import XMonad.Util.EZConfig(additionalKeys)
import XMonad.Layout.SimpleFloat

-- provides smartBorders for e.g. mplayer
import XMonad.Layout.NoBorders

-- trying to get flash full screen
import XMonad.Hooks.ManageHelpers

import System.IO

import qualified XMonad.StackSet as W

myManageHooks = manageDocks <+> composeAll
    -- get full screen on things like flash in firefox
    -- Allows focusing other monitors without killing the fullscreen
    [ isFullscreen --> (doF W.focusDown <+> doFullFloat) ]
    --  
    --  -- Single monitor setups, or if the previous hook doesn't work
    --      [ isFullscreen --> doFullFloat
    --          -- skipped
    --      ]

-- myLayouts = Tall ||| Full ||| simpleFloat

--        , handleEventHook = mconcat
--            [ docksEventHook
--            , handleEventHook defaultConfig ]
main = do 
	xmproc <- spawnPipe "/usr/bin/xmobar /home/esheldon/.xmonad/xmobarrc"
	xmonad $ defaultConfig {
       handleEventHook = mconcat
           [ docksEventHook
           , handleEventHook defaultConfig ]
        ,manageHook = myManageHooks <+> manageHook defaultConfig,
                   layoutHook = avoidStruts $ smartBorders $ layoutHook defaultConfig,
                   logHook = dynamicLogWithPP $ xmobarPP { 
                       ppOutput = hPutStrLn xmproc,
                       ppTitle = xmobarColor "green" "" . shorten 50
                   },
                   terminal = "xfce4-terminal"
    }`additionalKeys` myKeyBindings


--myWorkspaces = ["1","2","3","4","5","6","7","8","9"]
-- newer versions of dmenu are for some reason not recognized automatically,
-- so put it here explicitly.  Also we can control the color :)
myKeyBindings = [((mod1Mask, xK_p), spawn "dmenu_run -nb black -nf white")]
--    ++ 
--    [((m .|. mod1Mask, k), windows $ f i) -- Replace 'mod1Mask' with your mod key of choice.
--    | (i, k) <- zip myWorkspaces [xK_1 .. xK_9]
--    , (f, m) <- [(W.view, 0), (W.shift, shiftMask)]]

