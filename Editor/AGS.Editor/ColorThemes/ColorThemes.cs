﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace AGS.Editor
{
    public class ColorThemes
    {
        private readonly List<ColorTheme> _themes;
        private readonly EditorPreferences _preferences;

        private ColorTheme _current;

        public ColorThemes()
        {
            _themes = new List<ColorTheme>();
            _preferences = Factory.AGSEditor.Preferences;

            if (!Directory.Exists(DiskDir))
            {
                Directory.CreateDirectory(DiskDir);
            }

            Load();
        }

        public ColorTheme Current
        {
            get
            {
                return _current;
            }

            set
            {
                if (value == null)
                {
                    throw new NullReferenceException($"{Current} cannot be null.");
                }

                _current = value;
                _preferences.ColorTheme = Current.Name;
                Factory.AGSEditor.Preferences.SaveToRegistry();
            }
        }

        public IEnumerable<ColorTheme> Themes => _themes;

        public bool IsCurrentDefault => Current == ColorThemeStub.DEFAULT;

        private static string DiskDir => string.Format("{0}{1}AGS{1}Themes{1}", Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), Path.DirectorySeparatorChar);

        public void Load()
        {
            _themes.Clear();
            _themes.Add(ColorThemeStub.DEFAULT);
            Directory.GetFiles(DiskDir, "*.json").ToList().ForEach(f => _themes.Add(new ColorThemeJson(Path.GetFileNameWithoutExtension(f), f)));
            Current = _themes.FirstOrDefault(t => t.Name == Factory.AGSEditor.Preferences.ColorTheme) ?? ColorThemeStub.DEFAULT;
        }

        public void Apply(Action<ColorTheme> apply)
        {
            if (!IsCurrentDefault)
            {
                apply.Invoke(Current);
            }
        }

        public void Import(string dir)
        {
            string newDir = DiskDir + Path.GetFileName(dir);
            _themes.Add(new ColorThemeJson(Path.GetFileNameWithoutExtension(dir), dir));
            File.Copy(dir, newDir);
        }
    }
}
