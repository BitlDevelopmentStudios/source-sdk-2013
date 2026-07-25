using Microsoft.Win32;
using System;
using System.ComponentModel;
using System.IO;
using System.IO.Compression;
using System.Text.Json;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;

namespace LegacyModInstaller.App
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region ModData + variables
        
        private class ModData
        {
            public string ModName { get; set; }
            public string ModInstallDir { get; set; }
            public string AdditionalInstructions {  get; set; }
        }

        private ModData data;
        private string ModJsonFile = AppDomain.CurrentDomain.BaseDirectory + "data/mod.json";
        private string ModLogoFile = AppDomain.CurrentDomain.BaseDirectory + "data/logo.png";
        private string ModZipFile = AppDomain.CurrentDomain.BaseDirectory + "data/mod.zip";
        #endregion

        #region Window Logic
        public MainWindow()
        {
            InitializeComponent();
            MouseDown += Window_MouseDown;
        }

        private void window_loaded(object sender, RoutedEventArgs e)
        {
            if (!File.Exists(ModZipFile))
            {
                Close();
            }

            if (File.Exists(ModLogoFile))
            {
                Uri uri = new Uri(ModLogoFile, UriKind.RelativeOrAbsolute);
                Logo.Source = BitmapFrame.Create(uri);
            }
            else
            {
                Close();
            }

            if (File.Exists(ModJsonFile))
            {
                string jsonString = File.ReadAllText(ModJsonFile);
                data = JsonSerializer.Deserialize<ModData>(jsonString);
            }
            else
            {
                Close();
            }
        }

        private void Window_init(object sender, EventArgs e)
        {
        }

        private void window_closing(object sender, CancelEventArgs e)
        {
        }

        private void Window_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
                DragMove();
        }
        #endregion

        #region Launcher Logic
        private void close_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void minmize_Click(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        public static void CreateMessageBox(string text)
        {
            CustomMessageBox box = new CustomMessageBox(text);
            box.ShowDialog();
        }

        private void install_Click(object sender, RoutedEventArgs e)
        {
            RegistryKey localKey;

            if (Environment.Is64BitOperatingSystem)
            {
                localKey = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)
                    .OpenSubKey(@"SOFTWARE\\Wow6432Node\\Valve\\Steam");
            }
            else
            {
                localKey = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32)
                    .OpenSubKey(@"SOFTWARE\\Valve\\Steam");
            }

            string SteamInstallDir = localKey.GetValue("InstallPath").ToString();

            string SourceModsDir = SteamInstallDir + @"\steamapps\sourcemods";
            string InstallDir = SourceModsDir + @"\" + data.ModInstallDir;
            bool isUpdate = false;

            if (Directory.Exists(InstallDir))
            {
                CreateMessageBox($"{data.ModName} has been detected in your SourceMods folder. The installer will update {data.ModName} to the latest version by deleting the old version and extracting the new one. MAKE SURE YOU HAVE BACKED UP ANY ADDONS OR CUSTOM CONTENT.");
                Directory.Delete(InstallDir, true);
                isUpdate = true;
            }

            ZipFile.ExtractToDirectory(ModZipFile, SourceModsDir);

            while (!Directory.Exists(InstallDir))
            {
                Task.Delay(5);
                if (Directory.Exists(InstallDir))
                {
                    break;
                }
            }

            CreateMessageBox($"{data.ModName} has been installed to " + InstallDir + "! " + data.AdditionalInstructions);

            Close();
        }
        #endregion
    }
}
